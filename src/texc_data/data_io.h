#pragma once

/**
 * @brief Converts the given columns of sqlite table to csv string
 *
 * @param columns The columns of sqlite table
 * @param select_condition The conditions for the select query
 * @return csv string on success else NULL
 */
char *data_io_expandtexts_as_csv(const char *columns,
                                 const char *select_condition);

/**
 * @brief Saves all ExpandText belonging to the specified group as csv file
 *
 * @param group The group of the ExpandText
 */
void data_io_save_group(const char *group);

/**
 * @brief Saves all the ExpandTexts as csv files
 */
void data_io_save();

/**
 * @brief Loads all the ExpandTexts from csv file
 */
void data_io_load();
