#ifndef SCHEMA_C
#define SCHEMA_C
/*
 * Your schema migrations and setup, these will be automatically
 * run at startup. Your schema version will be picked up and
 * loaded when you init your app in app.c
 * Any call to schema checks the app->schema variable a against
 * the number in schema.version, if it is < it does nothing
 * if it is greater than, it executes the query, and on success
 * writes the new version to schema.version
 * 
 * The initial value of app->schema is 0
 * */
schema_table(&app,"people");
    schema_add_col(&app,"first_name","varchar(55)");
schema_end_table(); // This function writes the version out, must
                    // be called before incrementing app->schema++
app->schema++;
#endif
