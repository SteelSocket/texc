# Info

In texc all the text-expansions are stored in a sqlite database in memory and in a table called `expandtexts`. Certain commands like `texc list` optionaly can accept sql columns or conditions which can modify the return value of the command.

# Table Structure

| column  |          type          |      constraints       |                    detail                    |
| :-----: | :--------------------: | :--------------------: | :------------------------------------------: |
|  match  |          TEXT          |        NOT NULL        |                The match text                |
| expand  |          TEXT          |        NOT NULL        |               The expand text                |
|   id    |        INTEGER         |    NOT NULL UNIQUE     |     The unique id of the text-expansion      |
| enabled | INTEGER (only 0 and 1) |        NOT NULL        | Whether the text-expansion is enabled or not |
|  group  |          TEXT          | NOT NULL DEFAULT 'all' |       The group of the text expansion        |

Note: Since `group` is a keyword in sql we need to surround it with quotes in order to specify the column: `"group"`