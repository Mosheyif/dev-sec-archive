# Distributed Log Analyzer

Reads multiple server log files in parallel, applies pluggable analysis
strategies, and writes a JSON report.

---

## Design Patterns

| Pattern | Class | Role |
|---|---|---|
| Factory | `LogAnalyzerFactory` | Creates analyzer instances by type name; extensible by registering new entries |
| Strategy | `LogAnalyzer` | Interchangeable analysis algorithms — count levels, find common source, detect anomalies |
| Singleton | `ConfigurationManager` | Single shared configuration instance across the application |

---

## Analysis Modes

Configure via `properties.config`:

| Mode key | Description |
|---|---|
| `COUNT_LEVELS` | Tallies entries by log level (INFO, ERROR, WARNING, etc.) |
| `FIND_COMMON_SOURCE` | Identifies the most frequent log source across all files |
| `DETECT_ANOMALIES` | Flags error spikes within a sliding time window |

---

## Configuration

Key properties in `properties.config`:

| Property | Default | Description |
|---|---|---|
| `log.directory` | `logs` | Directory containing log files to process |
| `thread.pool.size` | `5` | Worker thread count |
| `output.file` | `output.json` | Path for the JSON report |
| `log.analysis` | `COUNT_LEVELS` | Analysis mode |
| `log.analysis.anomalies.levels` | `ERROR` | Levels to watch for anomaly detection |
| `log.analysis.anomalies.window` | `30` | Sliding window in seconds |
| `log.analysis.anomalies.threshold` | `2` | Spike threshold within the window |
