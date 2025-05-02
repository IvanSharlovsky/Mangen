# mangen

`mangen` is a command-line utility for generating a file manifest from a directory, including file hashes, with support for verification and exclusion filters.

## Features

- Recursively traverses directories and lists file paths with hash values
- Simple hash algorithm (based on Adler-32)
- Exclude files/directories by exact name (`-e`) or pattern (`-E`)
- Appends checksum to manifest for integrity checking
- `--verify` mode to validate the manifest
- Version output shows current Git commit

## Usage

```bash
./bin/mangen [DIR_PATH] [OPTIONS]
```

If `DIR_PATH` is omitted, the current directory is used.

## Options

- `-h`              Show help message and exit
- `-v`              Show Git commit hash and exit
- `-e NAME`         Exclude file or directory with exact name
- `-E PATTERN`      Exclude entries matching pattern (`*` = any sequence, `.` = any single char)
- `--verify FILE`   Verify integrity of a manifest file

## Output Format

Each line in the manifest:

```text
<relative/path/to/file> : <HASH>
```

Final line:

```text
Manifest checksum: <CHECKSUM>
```

## Example

```bash
./bin/mangen example_dir -e temp.log -E "*.sh" > manifest.txt
cat manifest.txt
./mangen --verify manifest.txt
```

## Test Cases

The utility is tested using `test.sh` via `make test`:

- **Test 1**: Basic manifest generation with all files
- **Test 2**: Exclusion by name using `-e`
- **Test 3**: Exclusion by pattern using `-E`
- **Test 4**: Git commit format check via `-v`
- **Test 5**: Verification of valid manifest via `--verify`
- **Test 6**: Detection of corrupted manifest via `--verify`

## Build

Use `make` to build the utility:

```bash
make           # Compile to ./bin/mangen
make clean     # Remove build artifacts
make version   # Show current Git commit hash
make test      # Run test suite using ./bin/mangen
```

## Author

Ivan Sharlovskii
