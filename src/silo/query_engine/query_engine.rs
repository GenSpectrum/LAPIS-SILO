use std::sync::Arc;
use std::sync::Mutex;
use std::time::Instant;

use crate::common::block_timer::BlockTimer;
use crate::database::Database;
use crate::query_engine::filter_expressions::expression::Expression;
use crate::query_engine::operator_result::OperatorResult;
use crate::query_engine::operators::operator::Operator;
use crate::query_engine::query::Query;
use crate::query_engine::query_result::QueryResult;

pub struct QueryEngine<'a> {
    database: &'a Database,
}

impl<'a> QueryEngine<'a> {
    pub fn new(database: &'a Database) -> Self {
        Self { database }
    }

    pub fn execute_query(&self, query_string: &str) -> QueryResult {
        let query = Query::new(query_string);

        log::debug!("Parsed query: {}", query.filter.to_string());

        let compiled_queries: Arc<Mutex<Vec<String>>> = Arc::new(Mutex::new(vec![String::new(); self.database.partitions.len()]));
        let partition_filters: Arc<Mutex<Vec<OperatorResult>>> = Arc::new(Mutex::new(vec![OperatorResult::default(); self.database.partitions.len()]));

        let filter_time = {
            let timer = Instant::now();
            rayon::scope(|s| {
                for partition_index in 0..self.database.partitions.len() {
                    let compiled_queries = Arc::clone(&compiled_queries);
                    let partition_filters = Arc::clone(&partition_filters);
                    let query_filter = query.filter.clone();
                    let database = self.database;
                    s.spawn(move |_| {
                        let part_filter = query_filter.compile(
                            database,
                            &database.partitions[partition_index],
                            Expression::AmbiguityMode::None,
                        );
                        compiled_queries.lock().unwrap()[partition_index] = part_filter.to_string();
                        partition_filters.lock().unwrap()[partition_index] = part_filter.evaluate();
                    });
                }
            });
            timer.elapsed().as_micros()
        };

        for (i, compiled_query) in compiled_queries.lock().unwrap().iter().enumerate() {
            log::debug!("Simplified query for partition {}: {}", i, compiled_query);
        }

        let action_time = {
            let timer = Instant::now();
            let query_result = query.action.execute_and_order(self.database, partition_filters.lock().unwrap().clone());
            timer.elapsed().as_micros()
        };

        log::info!("Query: {}", query_string);
        log::info!("Execution (filter): {} microseconds", filter_time);
        log::info!("Execution (action): {} microseconds", action_time);

        query_result
    }
}
