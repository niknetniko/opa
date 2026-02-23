# Guidelines

See the [architecture.md](architecture.md) file for data access.

# Tool Usage Guidelines

- **ALWAYS** use the built-in `read_file` tool to inspect file contents.
- **NEVER** use shell commands like `cat`, `less`, `head`, or `tail` to read files.
- **NEVER** use `grep` or `find` for file search; use the built-in codebase search tool.
- If you need to read a file, invoke the IDE tool directly.
