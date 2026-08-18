# RestgreSQL

RestgreSQL is a lightweight C++ REST API server that provides a JSON interface for interacting with a PostgreSQL database. It is designed to simplify data access for web applications by exposing database queries as RESTful endpoints.

## Features

- **JSON API**: Seamlessly interact with your PostgreSQL data using standard JSON.
- **PostgreSQL Integration**: Built on top of `libpqxx` for robust database communication.
- **SSL Support**: Secure your API communications with SSL/TLS.
- **Simple Configuration**: Uses a JSON configuration file for easy setup of database connections and SSL certificates.
- **Automatic Setup**: Automatically generates a default configuration file if one is not found.

## Prerequisites

Depending on your operating system, you will need the following dependencies:

### Arch Linux
```bash
sudo pacman -S boost git libpqxx openssl cmake
```

### Arch Linux (Testing)
To run the containerized test suite on Arch Linux, you will need:
```bash
sudo pacman -S docker docker-compose uv
```
Then, configure your user and start the service:
```bash
sudo usermod -aG docker $USER
sudo systemctl enable --now docker
```
To apply the group changes to your current terminal session without rebooting, run:
```bash
newgrp docker
```

### Ubuntu Server 24.04 LTS
```bash
sudo apt update
sudo apt install cmake libboost-all-dev libssl-dev libpqxx-dev libbz2-dev liblzma-dev libzstd-dev
```

## Installation & Build

1. **Clone the repository:**
   ```bash
   git clone <repository-url>
   cd restgresql
   ```

2. **Build the project using CMake:**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
   *Note: CMake will automatically fetch and build several dependencies (such as Boost, libpqxx, and Mongoose). However, system-level tools like `cmake`, `git`, and `openssl` are still required.*

## Usage

### Running the Server

By default, RestgreSQL looks for a configuration file at `/etc/restgresql/restgresql.conf` and listens on port `8000`. You can specify a custom configuration file using the `--config` flag.

```bash
./restgresql --config /path/to/your/config.json
```

### Development Mode (No SSL)

For local development, you can disable SSL using the `--dev-no-ssl` flag:

```bash
./restgresql --dev-no-ssl --config /path/to/your/config.json
```

## PostgreSQL Configuration Guide

To use RestgreSQL, you need a properly configured PostgreSQL database. Below is a guide for setting up a production-ready environment on Ubuntu.

### 1. Install and Configure PostgreSQL

First, install PostgreSQL and the client packages:

```bash
sudo apt-get install postgresql postgresql-client-common
```

Enable and start the service:

```bash
sudo systemctl enable --now postgresql
```

### 2. Set Up Database User and Permissions

Log in as the `postgres` user:

```bash
sudo -u postgres psql
```

Set a secure password for the `postgres` user:

```sql
ALTER USER postgres WITH ENCRYPTED PASSWORD 'yoursecurepasswordhere';
```

Exit `psql`:

```sql
\q
```

Next, configure PostgreSQL to use `scram-sha-256` authentication. Edit your `pg_hba.conf` file (e.g., `/etc/postgresql/16/main/pg_hba.conf`) and comment out the following lines to ensure password-encrypted connections:

```text
#local all postgres peer
#local all all peer
```

Restart PostgreSQL:

```bash
sudo systemctl restart postgresql
```

### 3. Create Database and Schema

Log in via SSL to your local server:

```bash
psql --host=127.0.0.1 --username=postgres --password
```

Run the following commands to set up your database, schema, and a read-only user for RestgreSQL:

```sql
-- Create the production database
CREATE DATABASE mysite;

-- Create the production schema
CREATE SCHEMA content;

-- Create a read-only user for RestgreSQL
CREATE ROLE restgresql WITH LOGIN PASSWORD 'yoursecurepasswordhere';

-- Grant permissions
GRANT CONNECT ON DATABASE mysite TO restgresql;
GRANT USAGE ON SCHEMA content TO restgresql;
GRANT SELECT ON ALL TABLES IN SCHEMA content TO restgresql;
ALTER DEFAULT PRIVILEGES IN SCHEMA content GRANT SELECT ON TABLES TO restgresql;
```

### 4. Database Schema and Views

RestgreSQL expects a specific schema structure. You can create the necessary tables and a view to simplify queries:

```sql
-- Create tables
CREATE TABLE "content".games ( game_id int4 NOT NULL, game_path varchar NULL, CONSTRAINT games_pkey PRIMARY KEY (game_id) );
CREATE TABLE "content".images ( image_id int4 NOT NULL, image_file_name varchar NOT NULL, image bytea NOT NULL, CONSTRAINT images_image_file_name_key UNIQUE (image_file_name), CONSTRAINT images_pkey PRIMARY KEY (image_id) );
CREATE TABLE "content".post_categories ( post_category_id int4 NOT NULL, post_category varchar NULL, CONSTRAINT post_categories_pkey PRIMARY KEY (post_category_id) );
CREATE TABLE "content".posts ( post_id int4 NOT NULL, game_id int4 NULL, post_category_id int4 NULL, post_uid varchar NULL, post_title varchar NULL, post_subtext varchar NULL, post_preview_image_url varchar NULL, post_datetime timestamp DEFAULT now() NULL, post_author varchar NULL, post_markdown_content text NULL, post_js_resource_key varchar NULL, CONSTRAINT posts_pkey PRIMARY KEY (post_id) );

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
```

## Configuration

The configuration file is a JSON object containing the following fields:

- `dbConnString`: The PostgreSQL connection string.
- `certPath`: The filesystem path to your server's SSL certificate file.
- `keyPath`: The filesystem path to your server's SSL key file.

Example `config.json`:

```json
{
  "dbConnString": "postgresql://user:password@localhost:5432/dbname",
  "certPath": "/etc/ssl/certs/server.crt",
  "keyPath": "/etc/ssl/private/server.key"
}
```

If the configuration file does not exist, RestgreSQL will prompt you to enter these details interactively upon startup.

## API Endpoints

The API provides several endpoints to retrieve data from your PostgreSQL database. All requests return JSON unless otherwise specified.

| Endpoint | Description | Parameter |
| :--- | :--- | :--- |
| `/api/posts` | Retrieve all posts | None |
| `/api/posts/{id}` | Retrieve a specific post by ID | `id` (integer) |
| `/api/recentPosts/{count}` | Retrieve the most recent posts | `count` (integer) |
| `/api/articles` | Retrieve all articles | None |
| `/api/recentArticles/{count}` | Retrieve the most recent articles | `count` (integer) |
| `/api/projects` | Retrieve all projects | None |
| `/api/recentProjects/{count}` | Retrieve the most recent projects | `count` (integer) |
| `/api/images/{id}` | Retrieve an image by ID (Binary) | `id` (integer) |
| `/api/images/{filename}` | Retrieve an image by filename (Binary) | `filename` (string) |

## Testing

The project includes a containerized test suite to verify API functionality.

### Running Tests

1. **Start the environment**:
   ```bash
   docker compose up -d
   ```

2. **Run the test suite using uv**:
   ```bash
   cd tests/integration
   uv run python test_suite.py
   ```

3. **Clean up**:
   ```bash
   docker compose down
   ```

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
