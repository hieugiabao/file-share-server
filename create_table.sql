-- SQLite

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
  FOREIGN KEY (user_id) REFERENCES users(id)
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
  FOREIGN KEY (owner_id) REFERENCES users(id)
);

-- create table `group_members` if not exists
CREATE TABLE IF NOT EXISTS group_members (
  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  group_id INTEGER NOT NULL,
  user_id INTEGER NOT NULL,
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (group_id) REFERENCES groups(id),
  FOREIGN KEY (user_id) REFERENCES users(id),
  UNIQUE (group_id, user_id)
);