create
or replace table preprocessing_table as
select *
from 'sample.ndjson.zst';

create view metadata_table as
select metadata.*
from preprocessing_table;

-- Creating partition_keys table -> used for partitioning
create
or replace table partition_keys as
select row_number() over () - 1 as id, *
from (SELECT pangoLineage as partition_key, COUNT(*) as count
      FROM preprocessing_table
      GROUP BY pangoLineage
      ORDER BY pangoLineage);


-- Calculate partitioning
create
or replace table partitioning as
with recursive allowed_count(allowed_count) as (select count(*) / 10 from partition_keys),
               grouped_partition_keys(from_id, to_id, count) as
                   (select id, id, count
                    from partition_keys
                    where id = 1
                    union all
                    select case when l1.count <= allowed_count then l1.from_id else l2.id end,
                           l2.id,
                           case
                               when l1.count <= allowed_count
                                   then l1.count + l2.count
                               else l2.count end
                    from grouped_partition_keys_rec l1,
                         partition_keys l2,
                         allowed_count
                    where l1.to_id + 1 = l2.id)
select row_number() over () - 1 as partition_id, *
from (select from_id, max(to_id) as to_id, max(count) as count
      from grouped_partition_keys
      group by from_id);


-- Partitioning metadata

create
or replace view partitioned_metadata as
select partitioning.partition_id as partition_id, metadata_table.*
from partition_keys,
     partitioning,
     metadata_table
where metadata_table.partition_key = partition_keys.partition_key
  AND partition_keys.id >= partitioning.from_id
  AND partition_keys.id <= partitioning.to_id;

-- Partition metadata without partitioning
create
or replace view partitioned_metadata as
select 0 as partition_id, metadata_table.*
from metadata_table;

select *
from partitioned_metadata;

copy partitioned_metadata to 'partitions' (FORMAT PARQUET, PARTITION_BY (partition));

select *
from read_parquet('partitions/*/*.parquet', hive_partitioning = 1);
