# Path
http://127.0.0.1:{port}/list?token={TOKEN}

# Usage

Returns all text expansion in csv format. See [sql.md](../sql.md) for all the columns

# Parameters

| parameter |  type  |       description       |               default               |
| :-------: | :----: | :---------------------: | :---------------------------------: |
|  columns  | string |  The columns to select  | match, expand, id, enabled, "group" |
| condition | string | The sql WHERE condition |                NULL                 |