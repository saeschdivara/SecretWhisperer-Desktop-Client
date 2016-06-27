CREATE TABLE IF NOT EXISTS contact (
    user_name CHAR(255) NOT NULL,
    public_key TEXT NOT NULL,
    connected_user CHAR(255) NOT NULL
);
