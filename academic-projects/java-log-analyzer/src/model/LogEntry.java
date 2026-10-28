/*********************************************************
*
* Description: Data model for a single parsed log entry.
* Author: Moshe Yifrah
*
*********************************************************/

package model;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

/**
 * Represents a single log entry from a log file.
 */
public class LogEntry {
    private LocalDateTime timestamp;
    private String level;
    private String source;
    private String message;
    private String originalLine;
    
    private static final DateTimeFormatter FORMATTER = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
    
    /**
     * Constructor for creating a log entry.
     * 
     * @param timestamp Timestamp of the log entry.
     * @param level Log level (INFO, ERROR, WARNING, etc.).
     * @param source Source of the log entry.
     * @param message The actual log message.
     * @param originalLine The original line from the log file.
     */
    public LogEntry(LocalDateTime timestamp, String level, String source, String message, String originalLine) {
        this.timestamp = timestamp;
        this.level = level;
        this.source = source;
        this.message = message;
        this.originalLine = originalLine;
    }
    
    /**
     * Gets the timestamp of the log entry.
     * 
     * @return The timestamp.
     */
    public LocalDateTime getTimestamp() {
        return timestamp;
    }
    
    /**
     * Gets the formatted timestamp string of the log entry.
     * 
     * @return The formatted timestamp string.
     */
    public String getFormattedTimestamp() {
        return timestamp.format(FORMATTER);
    }
    
    /**
     * Gets the log level.
     * 
     * @return The log level.
     */
    public String getLevel() {
        return level;
    }
    
    /**
     * Gets the source of the log entry.
     * 
     * @return The source.
     */
    public String getSource() {
        return source;
    }
    
    /**
     * Gets the log message.
     * 
     * @return The log message.
     */
    public String getMessage() {
        return message;
    }
    
    /**
     * Gets the original log line.
     * 
     * @return The original log line.
     */
    public String getOriginalLine() {
        return originalLine;
    }
    
    /**
     * Returns a string representation of the log entry.
     * 
     * @return A string representation of this log entry.
     */
    @Override
    public String toString() {
        return "[" + getFormattedTimestamp() + "] [" + level + "] [" + source + "] [" + message + "]";
    }
}