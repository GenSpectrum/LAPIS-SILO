create
or replace table preprocessing_table as
select *
from 'sample.ndjson.zst';

create view metadata_table as
select metadata.*
from preprocessing_table;

DROP TABLE lineages;

CREATE TABLE lineages AS
select *, row_number() over () as id
from (SELECT pangoLineage, COUNT(*) as count FROM preprocessing_table GROUP BY pangoLineage ORDER BY pangoLineage);
drop table partitioning;
create table partitioning as
with recursive allowed_count(allowed_count) as (select count(*) / 10 from lineages),
               grouped_lineages(from_id, to_id, count) as (select id, id, count
                                                           from lineages
                                                           where id = 1
                                                           union all
                                                           select case when l1.count <= allowed_count then l1.from_id else l2.id end,
                                                                  l2.id,
                                                                  case
                                                                      when l1.count <= allowed_count
                                                                          then l1.count + l2.count
                                                                      else l2.count end
                                                           from grouped_lineages l1,
                                                                lineages l2,
                                                                allowed_count
                                                           where l1.to_id + 1 = l2.id)
select *, row_number() over () as partition_id
from (select from_id, max(to_id) as to_id, max(count) as count
      from grouped_lineages
      group by from_id);

create view partitioned_metadata as
select partitioning.partition_id as partition, metadata_table.*
from lineages,
     partitioning,
     metadata_table
where metadata_table.pangoLineage = lineages.pangoLineage
  AND lineages.id >= partitioning.from_id
  AND lineages.id <= partitioning.to_id;

select *
from partitioned_metadata;
copy partitioned_metadata to 'partitions' (FORMAT PARQUET, PARTITION_BY (partition));

select *
from read_parquet('partitions/*/*.parquet', hive_partitioning = 1);
