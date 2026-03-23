CREATE TYPE adsk_object_status AS ENUM (
    'UPLOAD_AWAITING',
    'UPLOADING',
    'UPLOAD_FAILED',
    'CONVERSION_AWAITING',
    'CONVERTING',
    'CONVERSION_FAILED',
    'ACTIVE',
    'DELETED'
);

CREATE TABLE viewer_folders (
    id SERIAL PRIMARY KEY,
    parent_id INTEGER REFERENCES viewer_folders(id) ON DELETE CASCADE,
    name TEXT NOT NULL,
    associated_bucket_key TEXT DEFAULT 'NONE',
    path_from_root TEXT UNIQUE NOT NULL,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    thumbnail_path_from_root TEXT DEFAULT 'NONE'
);

CREATE TABLE viewer_oss_objects (
    id SERIAL PRIMARY KEY,
    file_name TEXT,
    file_exists BOOLEAN DEFAULT true,
    path_from_root TEXT UNIQUE,
    folder_id INTEGER REFERENCES viewer_folders(id) ON DELETE SET NULL,
    bucket_key TEXT,
    object_key TEXT UNIQUE,
    object_id TEXT, 
    sha1 CHAR(40),
    size BIGINT,
    location TEXT,
    status adsk_object_status DEFAULT 'UPLOAD_AWAITING',
    last_modified TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT CURRENT_TIMESTAMP,
    uploaded_at TIMESTAMPTZ,
    deleted_at TIMESTAMPTZ
);

CREATE INDEX idx_oss_active_objects 
ON viewer_oss_objects(object_key, bucket_key) 
WHERE status != 'DELETED';

CREATE INDEX idx_oss_objects_folder_id ON viewer_oss_objects(folder_id);
CREATE INDEX idx_folders_parent_id ON viewer_folders(parent_id);

CREATE OR REPLACE FUNCTION handle_viewer_object_cleanup()
RETURNS TRIGGER AS $$
BEGIN
    IF NEW.file_exists = false THEN
        NEW.path_from_root := NULL;
    END IF;

    IF NEW.status = 'DELETED' AND (OLD.status IS NULL OR OLD.status != 'DELETED') THEN
        NEW.object_key := NULL;
        NEW.deleted_at := CURRENT_TIMESTAMP;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_viewer_objects_cleanup
BEFORE UPDATE ON viewer_oss_objects
FOR EACH ROW
EXECUTE FUNCTION handle_viewer_object_cleanup();