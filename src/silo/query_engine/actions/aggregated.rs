use std::collections::HashMap;
use std::sync::Arc;
use std::sync::Mutex;

use crate::config::database_config::DatabaseConfig;
use crate::database::Database;
use crate::query_engine::actions::action::Action;
use crate::query_engine::actions::tuple::Tuple;
use crate::query_engine::operator_result::OperatorResult;
use crate::query_engine::query_parse_exception::QueryParseException;
use crate::query_engine::query_result::{QueryResult, QueryResultEntry};
use crate::storage::column_group::ColumnGroup;
use crate::storage::column_metadata::ColumnMetadata;
use crate::storage::sequence_store::SequenceStore;
use crate::storage::unaligned_sequence_store::UnalignedSequenceStore;

pub struct Aggregated {
    group_by_fields: Vec<String>,
    order_by_fields: Vec<OrderByField>,
}

impl Aggregated {
    pub fn new(group_by_fields: Vec<String>) -> Self {
        Self {
            group_by_fields,
            order_by_fields: Vec::new(),
        }
    }

    pub fn validate_order_by_fields(&self, database: &Database) {
        let field_metadata = parse_group_by_fields(database, &self.group_by_fields);

        for field in &self.order_by_fields {
            if field.name != COUNT_FIELD
                && !field_metadata.iter().any(|metadata| metadata.name == field.name)
            {
                panic!(
                    "The orderByField '{}' cannot be ordered by, as it does not appear in the groupByFields.",
                    field.name
                );
            }
        }
    }

    pub fn execute(
        &self,
        database: &Database,
        bitmap_filters: Vec<OperatorResult>,
    ) -> QueryResult {
        if self.group_by_fields.is_empty() {
            return aggregate_without_grouping(bitmap_filters);
        }

        let group_by_metadata = parse_group_by_fields(database, &self.group_by_fields);

        let mut tuple_maps: Vec<HashMap<Tuple, u32>> = Vec::new();
        let mut tuple_factories: Vec<TupleFactory> = Vec::new();

        for partition in &database.partitions {
            tuple_maps.push(HashMap::new());
            tuple_factories.push(TupleFactory::new(&partition.columns, &group_by_metadata));
        }

        let tuple_maps = Arc::new(Mutex::new(tuple_maps));
        let tuple_factories = Arc::new(Mutex::new(tuple_factories));

        rayon::scope(|s| {
            for partition_id in 0..database.partitions.len() {
                let tuple_maps = Arc::clone(&tuple_maps);
                let tuple_factories = Arc::clone(&tuple_factories);
                let bitmap = &bitmap_filters[partition_id];

                s.spawn(move |_| {
                    let mut tuple_maps = tuple_maps.lock().unwrap();
                    let mut tuple_factories = tuple_factories.lock().unwrap();

                    let tuple_factory = &mut tuple_factories[partition_id];
                    let map = &mut tuple_maps[partition_id];

                    for &index in bitmap.iter() {
                        let current_tuple = tuple_factory.allocate_one(index);
                        *map.entry(tuple_factory.copy_tuple(&current_tuple)).or_insert(0) += 1;
                    }
                });
            }
        });

        let mut final_map: HashMap<Tuple, u32> = HashMap::new();
        let tuple_factories = tuple_factories.lock().unwrap();
        let tuple_maps = tuple_maps.lock().unwrap();

        for partition_id in 0..database.partitions.len() {
            let tuple_factory = &tuple_factories[partition_id];
            let map = &tuple_maps[partition_id];

            for (tuple, value) in map {
                *final_map.entry(tuple_factory.copy_tuple(tuple)).or_insert(0) += value;
            }
        }

        QueryResult::from_vector(generate_result(final_map))
    }
}

fn parse_group_by_fields(
    database: &Database,
    group_by_fields: &[String],
) -> Vec<ColumnMetadata> {
    group_by_fields
        .iter()
        .map(|field| {
            database
                .database_config
                .get_metadata(field)
                .expect(&format!("Metadata field '{}' to group by not found", field))
        })
        .collect()
}

fn generate_result(tuple_counts: HashMap<Tuple, u32>) -> Vec<QueryResultEntry> {
    tuple_counts
        .into_iter()
        .map(|(tuple, count)| {
            let mut fields = tuple.get_fields();
            fields.insert(COUNT_FIELD.to_string(), count.into());
            QueryResultEntry { fields }
        })
        .collect()
}

fn aggregate_without_grouping(bitmap_filters: Vec<OperatorResult>) -> QueryResult {
    let count: u32 = bitmap_filters.iter().map(|filter| filter.cardinality()).sum();
    let mut tuple_fields = HashMap::new();
    tuple_fields.insert(COUNT_FIELD.to_string(), count.into());
    QueryResult::from_vector(vec![QueryResultEntry { fields: tuple_fields }])
}

const COUNT_FIELD: &str = "count";

impl From<&nlohmann::json::JsonValue> for Aggregated {
    fn from(json: &nlohmann::json::JsonValue) -> Self {
        let group_by_fields = json
            .get("groupByFields")
            .and_then(|v| v.as_array())
            .map(|arr| arr.iter().filter_map(|v| v.as_str().map(String::from)).collect())
            .unwrap_or_default();
        Aggregated::new(group_by_fields)
    }
}
