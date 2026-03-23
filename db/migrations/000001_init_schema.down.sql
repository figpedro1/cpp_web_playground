DROP TRIGGER IF EXISTS trg_viewer_objects_cleanup ON viewer_oss_objects;

DROP FUNCTION IF EXISTS handle_viewer_object_cleanup();

DROP INDEX IF EXISTS idx_folders_parent_id;
DROP INDEX IF EXISTS idx_oss_objects_folder_id;
DROP INDEX IF EXISTS idx_oss_active_objects;

DROP TABLE IF EXISTS viewer_oss_objects;
DROP TABLE IF EXISTS viewer_folders;

DROP TYPE IF EXISTS adsk_object_status;