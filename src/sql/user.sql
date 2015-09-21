CREATE TABLE IF NOT EXISTS user(
    user_name CHAR(255) NOT NULL,
    public_key TEXT NOT NULL,
    private_key TEXT NOT NULL,
    symmetric_key TEXT NOT NULL
);
