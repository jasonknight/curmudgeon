db_mysql(&app,_DB_HOST_,_DB_USER_,_DB_PASS_);
db_exec(&app, "CREATE DATABASE IF NOT EXISTS `%s`",_DB_NAME_,NULL);
db_select_db(&app,_DB_NAME_);
