-- SQLite
PRAGMA foreign_keys=ON;

-- create table `users` if not exists
CREATE TABLE IF NOT EXISTS users (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  display_name TEXT NOT NULL,
  username TEXT NOT NULL UNIQUE,
  password TEXT NOT NULL,
  status INTEGER NOT NULL DEFAULT 1
);

-- create table `sessions` if not exists
CREATE TABLE IF NOT EXISTS sessions (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  user_id INTEGER NOT NULL,
  token TEXT NOT NULL UNIQUE DEFAULT (hex(randomblob(32))),
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- create table `groups` if not exists
CREATE TABLE IF NOT EXISTS groups (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL UNIQUE,
  description TEXT NULL,
  avatar TEXT NULL,
  owner_id INTEGER NOT NULL,
  status INTEGER NOT NULL DEFAULT 1,
  -- uuid code generate
  code TEXT NOT NULL UNIQUE DEFAULT (hex(randomblob(5))),
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE
);

-- create table `group_members` if not exists
CREATE TABLE IF NOT EXISTS group_members (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  group_id INTEGER NOT NULL,
  user_id INTEGER NOT NULL,
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,
  FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
  UNIQUE (group_id, user_id)
);

-- create table `directories` if not exists
CREATE TABLE IF NOT EXISTS directories (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  permission INTEGER NOT NULL DEFAULT 1,
  path TEXT NOT NULL,
  parent_id INTEGER NULL,
  group_id INTEGER NOT NULL,
  owner_id INTEGER NOT NULL,
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (parent_id) REFERENCES directories(id) ON DELETE CASCADE,
  FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,
  FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE,
  UNIQUE (name, parent_id, group_id)
);

-- create table `files` if not exists
CREATE TABLE IF NOT EXISTS files (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  size INTEGER NOT NULL,
  permission INTEGER NOT NULL DEFAULT 1,
  path TEXT NOT NULL,
  directory_id INTEGER NULL,
  group_id INTEGER NOT NULL,
  owner_id INTEGER NOT NULL,
  modified_by INTEGER NULL,
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (directory_id) REFERENCES directories(id) ON DELETE CASCADE,
  FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,
  FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE,
  FOREIGN KEY (modified_by) REFERENCES users(id),
  UNIQUE(name, directory_id, group_id)
);

CREATE UNIQUE INDEX IF NOT EXISTS `idx_not_directory_id` ON files(name, group_id) 
WHERE directory_id IS NULL;