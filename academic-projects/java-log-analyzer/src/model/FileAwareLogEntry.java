/*********************************************************
*
* Description: Log entry model that tracks the source file it was parsed from.
* Author: Moshe Yifrah
*
*********************************************************/

package model;

import java.time.LocalDateTime;

/**
 * Extension of LogEntry that keeps track of which file it came from.
 * This allows for proper tracking of anomalies by file.
 */
public class FileAwareLogEntry extends LogEntry {
    private String fileName;
    
    /**
     * Constructor for creating a file-aware log entry.
     * 
     * @param timestamp Timestamp of the log entry.
     * @param level Log level (INFO, ERROR, WARNING, etc.).
     * @param source Source of the log entry.
     * @param message The actual log message.
     * @param originalLine The original line from the log file.
     */
    public FileAwareLogEntry(LocalDateTime timestamp, String level, String source, 
                          String message, String originalLine) {
        super(timestamp, level, source, message, originalLine);
        this.fileName = "";
    }
    
    /**
     * Gets the file name that this log entry came from.
     * 
     * @return The file name.
     */
    public String getFileName() {
        return fileName;
    }
    
    /**
     * Sets the file name that this log entry came from.
     * 
     * @param fileName The file name.
     */
    public void setFileName(String fileName) {
        this.fileName = fileName;
    }
}