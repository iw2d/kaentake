### Usage

Download all files from [Releases](https://github.com/iw2d/kaentake/releases) into a clean v83 installtion directory, then run Kaentake.exe as administrator.

A standalone WZ file (Custom.wz) is included to extend the system option dialog to include a combobox for selecting the resolution in game.

---

### Configuration

Either the commandline arguments or the `config.ini` file can be used to redirect the client to a host other than the default host: `"127.0.0.1"`.

Example for using commandline arguments, this will take precedence over the configuration file:
```
Kaentake.exe 127.0.0.1 8484
```

Example for using `config.ini`:
```
[config]
host=127.0.0.1
port=8484
```