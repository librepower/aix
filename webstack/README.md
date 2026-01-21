# Web Stack for AIX

**LibrePower - Unlocking Power Systems through open source**

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![PHP](https://img.shields.io/badge/PHP-8.3.16-777BB4)
![MariaDB](https://img.shields.io/badge/MariaDB-11.8.0-003545)
![PostgreSQL](https://img.shields.io/badge/PostgreSQL-18.0-336791)

Run modern web applications on AIX! Complete LAMP/LEMP stack with AI/vector search capabilities.

## Quick Start

```bash
# Add LibrePower repository
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install LAMP stack (Apache + PHP + MariaDB)
dnf install httpd php83 php83-fpm mariadb11

# Or LEMP stack (nginx + PHP + PostgreSQL)
dnf install nginx php83 php83-fpm postgresql18-server

# For AI/RAG applications with vector search
dnf install postgresql18-server postgresql18-pgvector
```

## Stack Components

| Component | Version | Source | Install |
|-----------|---------|--------|---------|
| **Apache httpd** | 2.4.66 | IBM AIX Toolbox | `dnf install httpd` |
| **nginx** | 1.27.4 | IBM AIX Toolbox | `dnf install nginx` |
| **PHP** | 8.3.16 | LibrePower | `dnf install php83 php83-fpm` |
| **MariaDB** | 11.8.0 | LibrePower | `dnf install mariadb11` |
| **PostgreSQL** | 18.0 | IBM AIX Toolbox | `dnf install postgresql18-server` |
| **pgvector** | 0.8.1 | IBM AIX Toolbox | `dnf install postgresql18-pgvector` |

## Compatible Applications

Tested and working on AIX:

| Application | Type | Database | Notes |
|-------------|------|----------|-------|
| **WordPress** | CMS/Blog | MariaDB | Full functionality |
| **Nextcloud** | File Sync | MariaDB/PostgreSQL | Recommended with nginx |
| **Flarum** | Forum | MariaDB | PHP 8.3 compatible |
| **Lychee** | Photo Gallery | MariaDB | Requires GD extension |
| **Kanboard** | Project Management | MariaDB/PostgreSQL | Lightweight |
| **Zabbix Frontend** | Monitoring | PostgreSQL | All extensions included |
| **Docling + RAG** | AI/Document Processing | PostgreSQL + pgvector | Vector similarity search |

---

## LAMP Setup (Apache + MariaDB)

### 1. Install Components

```bash
dnf install httpd php83 php83-fpm mariadb11
```

### 2. Configure PHP-FPM

```bash
# Edit /opt/freeware/etc/php-fpm.d/www.conf
# Change: listen = /var/run/php-fpm/www.sock
# Add: listen.owner = nobody
#      listen.group = nobody
```

### 3. Configure Apache

Create `/opt/freeware/etc/httpd/conf.d/php-fpm.conf`:

```apache
<FilesMatch \.php$>
    SetHandler "proxy:unix:/var/run/php-fpm/www.sock|fcgi://localhost"
</FilesMatch>

DirectoryIndex index.php index.html
```

### 4. Start Services

```bash
# Create socket directory
mkdir -p /var/run/php-fpm

# Start services
startsrc -s php-fpm
startsrc -s httpd
startsrc -s mariadb11

# Enable at boot (add to /etc/rc.local)
echo "startsrc -s php-fpm" >> /etc/rc.local
echo "startsrc -s httpd" >> /etc/rc.local
echo "startsrc -s mariadb11" >> /etc/rc.local
```

### 5. Secure MariaDB

```bash
mysql_secure_installation
```

---

## LEMP Setup (nginx + PostgreSQL)

### 1. Install Components

```bash
dnf install nginx php83 php83-fpm php83-pgsql postgresql18-server
```

### 2. Initialize PostgreSQL

```bash
# Initialize database cluster
/opt/freeware/bin/initdb -D /var/lib/pgsql/data

# Start PostgreSQL
startsrc -s postgresql

# Create user and database
sudo -u postgres createuser -P myapp
sudo -u postgres createdb -O myapp myapp_db
```

### 3. Configure nginx

Create `/opt/freeware/etc/nginx/conf.d/php.conf`:

```nginx
server {
    listen 80;
    server_name _;
    root /var/www/html;
    index index.php index.html;

    location / {
        try_files $uri $uri/ /index.php?$query_string;
    }

    location ~ \.php$ {
        fastcgi_pass unix:/var/run/php-fpm/www.sock;
        fastcgi_index index.php;
        fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
        include fastcgi_params;
    }
}
```

### 4. Start Services

```bash
startsrc -s php-fpm
startsrc -s nginx
startsrc -s postgresql
```

---

## AI/RAG with pgvector

PostgreSQL 18 with pgvector enables vector similarity search for AI applications like document retrieval (RAG), semantic search, and embeddings storage.

### Install pgvector

```bash
dnf install postgresql18-server postgresql18-pgvector
```

### Enable Extension

```sql
-- Connect to your database
psql -U postgres -d mydb

-- Enable pgvector
CREATE EXTENSION vector;

-- Create table with vector column
CREATE TABLE documents (
    id SERIAL PRIMARY KEY,
    content TEXT,
    embedding vector(1536)  -- OpenAI embedding dimension
);

-- Create index for fast similarity search
CREATE INDEX ON documents USING ivfflat (embedding vector_cosine_ops)
    WITH (lists = 100);
```

### Example: Document RAG with Docling

```python
import psycopg2
from pgvector.psycopg2 import register_vector

# Connect to PostgreSQL
conn = psycopg2.connect("dbname=ragdb user=postgres")
register_vector(conn)

# Store document embedding
cur = conn.cursor()
cur.execute(
    "INSERT INTO documents (content, embedding) VALUES (%s, %s)",
    (document_text, embedding_vector)
)

# Similarity search
cur.execute("""
    SELECT content, 1 - (embedding <=> %s) AS similarity
    FROM documents
    ORDER BY embedding <=> %s
    LIMIT 5
""", (query_embedding, query_embedding))

results = cur.fetchall()
```

### Use Cases

- **Document Q&A**: Build chatbots that answer questions from your documents
- **Semantic Search**: Find similar content across large document collections
- **Recommendation Systems**: Suggest related items based on embeddings

---

## PHP Extensions

LibrePower PHP 8.3 includes 53 extensions:

```
bcmath, bz2, calendar, ctype, curl, dba, dom, exif, fileinfo, filter,
ftp, gd, gettext, gmp, iconv, imap, intl, json, ldap, mbstring, mysqli,
mysqlnd, odbc, opcache, openssl, pcntl, pdo, pdo_mysql, pdo_odbc,
pdo_pgsql, pdo_sqlite, pgsql, phar, posix, readline, reflection,
session, shmop, simplexml, soap, sockets, sodium, spl, sqlite3,
standard, sysvmsg, sysvsem, sysvshm, tidy, tokenizer, xml, xmlreader,
xmlwriter, xsl, zip, zlib
```

---

## Performance Tuning

### PHP opcache (recommended)

Edit `/opt/freeware/etc/php.d/10-opcache.ini`:

```ini
opcache.enable=1
opcache.memory_consumption=256
opcache.interned_strings_buffer=16
opcache.max_accelerated_files=10000
opcache.revalidate_freq=2
```

### MariaDB

Edit `/opt/freeware/etc/my.cnf`:

```ini
[mysqld]
innodb_buffer_pool_size = 1G
innodb_log_file_size = 256M
query_cache_size = 64M
```

### PostgreSQL

Edit `/var/lib/pgsql/data/postgresql.conf`:

```ini
shared_buffers = 256MB
effective_cache_size = 1GB
maintenance_work_mem = 128MB
```

---

## Troubleshooting

### PHP-FPM won't start

```bash
# Check socket directory exists
mkdir -p /var/run/php-fpm

# Check permissions
chown nobody:nobody /var/run/php-fpm

# Check logs
cat /opt/freeware/var/log/php-fpm.log
```

### Permission denied on /var/www

```bash
# Set correct ownership
chown -R nobody:nobody /var/www/html

# Set permissions
chmod -R 755 /var/www/html
```

### Database connection refused

```bash
# Check service is running
lssrc -s mariadb11
lssrc -s postgresql

# Check port is listening
netstat -an | grep 3306   # MariaDB
netstat -an | grep 5432   # PostgreSQL
```

---

## Links

- [PHP 8.3 Documentation](../php83/)
- [MariaDB Documentation](https://gitlab.com/librepower/mariadb)
- [PostgreSQL pgvector](https://github.com/pgvector/pgvector)
- [LibrePower](https://librepower.org)

## License

Documentation: CC-BY-4.0

Components:
- PHP: PHP License
- MariaDB: GPL-2.0
- PostgreSQL: PostgreSQL License
- Apache httpd: Apache-2.0
- nginx: BSD-2-Clause
