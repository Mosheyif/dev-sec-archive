/*********************************************************
*
* Description: Parses raw log lines into structured LogEntry objects.
* Author: Moshe Yifrah
*
*********************************************************/

package parser;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeParseException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import model.FileAwareLogEntry;
import model.LogEntry;

/**
 * Parser for log files that follows the format: [TIMESTAMP] [LEVEL] [SOURCE] [MESSAGE]
 */
public class LogParser {
    
    // Regular expression to match the log entry format
    private static final String LOG_PATTERN = "\\[(.*?)\\]\\s*\\[(.*?)\\]\\s*\\[(.*?)\\]\\s*\\[(.*?)\\]";
    private static final Pattern pattern = Pattern.compile(LOG_PATTERN);
    private static final DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
    
    /**
     * Parses a log line and creates a LogEntry object.
     * 
     * @param line The log line to parse.
     * @return A LogEntry object if the parsing succeeds, null otherwise.
     */
    public static LogEntry parse(String line) {
        System.out.println("Parsing line: " + line);
        Matcher matcher = pattern.matcher(line);
        
        if (matcher.matches()) {
            String timestampStr = matcher.group(1).trim();
            String level = matcher.group(2).trim();
            String source = matcher.group(3).trim();
            String message = matcher.group(4).trim();
            
            System.out.println("Matched: timestamp=" + timestampStr + ", level=" + level + ", source=" + source);
            
            try {
                LocalDateTime timestamp = LocalDateTime.parse(timestampStr, formatter);
                System.out.println("Successfully parsed timestamp: " + timestamp);
                return new FileAwareLogEntry(timestamp, level, source, message, line);
            } catch (DateTimeParseException e) {
                System.out.println("Failed to parse timestamp: " + timestampStr + " - Error: " + e.getMessage());
                System.out.println("unexpected input " + line);
                return null;
            }
        } else {
            System.out.println("Line does not match expected format: " + line);
            System.out.println("unexpected input " + line);
            return null;
        }
    }
    
    /**
     * Parses a log line and creates a LogEntry object with file information.
     * 
     * @param line The log line to parse.
     * @param fileName The name of the file this log entry came from.
     * @return A FileAwareLogEntry object if the parsing succeeds, null otherwise.
     */
    public static LogEntry parse(String line, String fileName) {
        LogEntry entry = parse(line);
        
        if (entry instanceof FileAwareLogEntry) {
            ((FileAwareLogEntry) entry).setFileName(fileName);
        }
        
        return entry;
    }
}