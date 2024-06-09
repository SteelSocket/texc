# Info

After running the texc for the first time, a default settings.ini is generated in local appdata folder (On Windows: `Users\AppData\Local\texc\settings.ini`).  

Settings is loaded at the start of `texc`, For any changes in settings.ini to take effect please close and run texc again.  

# Settings

## general_settings

### log_level
- Type: str
- Values: INFO, WARN, ERROR
- Default: INFO
- Description: The log level for the logger

## expand_settings

### repeat_key_delay
- Type: int
- Values: Positive integer
- Default: 120
- Description: The time delay (in milliseconds) between repeated key presses of same character. Default is recommended as some applications do not register such fast repeated key presses.
