load json;
CREATE TABLE input2 AS SELECT docid, vector FROM '../msmarco-passage-openai-ada2/0.jsonl.gz';
CREATE TABLE queries AS SELECT qid, vector AS embedding FROM 'conformanceTests/anserini-tools/topics-and-qrels/topics.dl19-passage.openai-ada2.jsonl.gz';
SELECT list_cosine_similarity(input2.vector, queries.embedding) FROM input2 CROSS JOIN queries ORDER BY list_cosine_similarity(input2.vector, queries.embedding) LIMIT 5;