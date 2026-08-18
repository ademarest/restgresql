-- Create the production database and schema
CREATE SCHEMA content;

-- Create tables
CREATE TABLE "content".games ( 
    game_id int4 NOT NULL, 
    game_path varchar NULL, 
    CONSTRAINT games_pkey PRIMARY KEY (game_id) 
);

CREATE TABLE "content".images ( 
    image_id int4 NOT NULL, 
    image_file_name varchar NOT NULL, 
    image bytea NOT NULL, 
    CONSTRAINT images_image_file_name_key UNIQUE (image_file_name), 
    CONSTRAINT images_pkey PRIMARY KEY (image_id) 
);

CREATE TABLE "content".post_categories ( 
    post_category_id int4 NOT NULL, 
    post_category varchar NULL, 
    CONSTRAINT post_categories_pkey PRIMARY KEY (post_category_id) 
);

CREATE TABLE "content".posts ( 
    post_id int4 NOT NULL, 
    game_id int4 NULL, 
    post_category_id int4 NULL, 
    post_uid varchar NULL, 
    post_title varchar NULL, 
    post_subtext varchar NULL, 
    post_preview_image_url varchar NULL, 
    post_datetime timestamp DEFAULT now() NULL, 
    post_author varchar NULL, 
    post_markdown_content text NULL, 
    post_js_resource_key varchar NULL, 
    CONSTRAINT posts_pkey PRIMARY KEY (post_id) 
);

-- Create a view for easier querying
CREATE OR REPLACE VIEW "content".vw_posts AS 
SELECT p.post_id, p.game_id, p.post_category_id, p.post_uid, p.post_title, p.post_subtext, p.post_preview_image_url, p.post_datetime, p.post_author, p.post_markdown_content, p.post_js_resource_key, g.game_path, pc.post_category 
FROM content.posts p 
LEFT JOIN content.games g USING (game_id) 
LEFT JOIN content.post_categories pc USING (post_category_id);

-- Create helper functions for the API
CREATE OR REPLACE FUNCTION content.fn_get_image_by_id(prm_image_id integer) RETURNS SETOF content.images LANGUAGE sql AS $function$ select i.* from content.images i where i.image_id = prm_image_id $function$;
CREATE OR REPLACE FUNCTION content.fn_get_image_by_filename(prm_image_file_name character varying) RETURNS SETOF content.images LANGUAGE sql AS $function$ select i.* from content.images i where image_file_name = prm_image_file_name $function$;
CREATE OR REPLACE FUNCTION content.fn_get_post_by_id(prm_post_id integer) RETURNS SETOF content.vw_posts LANGUAGE sql AS $function$ select p.* from content.vw_posts p where p.post_id = prm_post_id $function$;
CREATE OR REPLACE FUNCTION content.fn_get_recent_posts(prm_how_many_posts integer) RETURNS SETOF content.vw_posts LANGUAGE sql AS $function$ select p.* from content.vw_posts p order by p.post_datetime desc limit prm_how_many_posts; $function$;
CREATE OR REPLACE FUNCTION content.fn_get_recent_articles(prm_how_many_articles integer) RETURNS SETOF content.vw_posts LANGUAGE sql AS $function$ select p.* from content.vw_posts p where p.post_category = 'article' order by p.post_datetime desc limit prm_how_many_articles; $function$;
CREATE OR REPLACE FUNCTION content.fn_get_recent_projects(prm_how_many_projects integer) RETURNS SETOF content.vw_posts LANGUAGE sql AS $function$ select p.* from content.vw_posts p where p.post_category = 'project' order by p.post_datetime desc limit prm_how_many_projects; $function$;

-- Seed data for testing
INSERT INTO "content".post_categories (post_category_id, post_category) VALUES (1, 'article'), (2, 'project');
INSERT INTO "content".games (game_id, game_path) VALUES (1, '/games/test_game');
INSERT INTO "content".posts (post_id, game_id, post_category_id, post_title, post_markdown_content) VALUES 
(1, 1, 1, 'Test Article', 'This is a test article content'),
(2, 1, 2, 'Test Project', 'This is a test project content');
INSERT INTO "content".images (image_id, image_file_name, image) VALUES (1, 'test.png', decode('89504e470d0a1a0a0000000d494844520000000100000001080200000089704e47', 'hex'));
