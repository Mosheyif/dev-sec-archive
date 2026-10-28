/*********************************************************
*
* Description: Log analyzer that counts entries by log level.
* Author: Moshe Yifrah
*
*********************************************************/

package analyzer.impl;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import analyzer.LogAnalyzer;
import model.LogEntry;
import org.json.JSONObject;

/**
 * Implements the Strategy pattern to analyze log entries and count occurrences of each log level.
 * This analyzer counts how many times each log level (INFO, WARNING, ERROR, etc.) appears
 * across all processed log files.
 * 
 * Thread-safety is ensured by using synchronized collections and blocks to handle
 * concurrent access from multiple threads during parallel log processing.
 */
public class LevelCountAnalyzer implements LogAnalyzer {
    
    private final Map<String, Integer> levelCounts;
    
    /**
     * Constructor initializing the level counts map.
     */
    public LevelCountAnalyzer() {
        levelCounts = Collections.synchronizedMap(new HashMap<>());
    }
    
    /**
     * Returns the name of this analyzer, used for identifying it in configuration
     * and output JSON reports.
     * 
     * @return The name identifier for this analyzer ("COUNT_LEVELS").
     */
    @Override
    public String getName() {
        return "COUNT_LEVELS";
    }
    
     /**
     * Processes a list of log entries to count occurrences of each log level.
     * This method is called for each batch of log entries from a file being processed.
     * All log levels are converted to lowercase for consistent counting.
     * 
     * @param logEntries The list of log entries to analyze.
     * @param properties Configuration properties (not used in this analyzer).
     */
    @Override
    public void analyze(List<LogEntry> logEntries, Properties properties) {
        System.out.println("LevelCountAnalyzer: Analyzing " + logEntries.size() + " entries");
        
        for (LogEntry entry : logEntries) {
            String level = entry.getLevel().toLowerCase();
            synchronized (levelCounts) {
                levelCounts.put(level, levelCounts.getOrDefault(level, 0) + 1);
            }
        }
        
        System.out.println("LevelCountAnalyzer: Counts after analysis: " + levelCounts);
    }
    
    /**
     * Returns the analysis results as a JSON object.
     * The JSON object contains key-value pairs where the key is the log level
     * and the value is the count of occurrences.
     * 
     * @return A JSONObject containing the count of each log level.
     */
    @Override
    public JSONObject getResults() {
        JSONObject result = new JSONObject();
        
        System.out.println("LevelCountAnalyzer: Getting results, counts: " + levelCounts);
        
        synchronized (levelCounts) {
            for (Map.Entry<String, Integer> entry : levelCounts.entrySet()) {
                result.put(entry.getKey(), entry.getValue());
            }
        }
        
        return result;
    }
    
    /**
     * Merges the results of this analyzer with results from another instance.
     * This is used when combining results from parallel processing of multiple files.
     * The counts from the other analyzer are added to the counts in this analyzer.
     * 
     * @param other Another instance of LogAnalyzer (must be LevelCountAnalyzer).
     */
    @Override
    public void mergeResults(LogAnalyzer other) {
        if (!(other instanceof LevelCountAnalyzer)) {
            return;
        }
        
        LevelCountAnalyzer otherAnalyzer = (LevelCountAnalyzer) other;
        
        System.out.println("LevelCountAnalyzer: Merging results");
        System.out.println("  - Current counts: " + levelCounts);
        System.out.println("  - Other counts: " + otherAnalyzer.levelCounts);
        
        synchronized (levelCounts) {
            synchronized (otherAnalyzer.levelCounts) {
                for (Map.Entry<String, Integer> entry : otherAnalyzer.levelCounts.entrySet()) {
                    String level = entry.getKey();
                    int count = entry.getValue();
                    levelCounts.put(level, levelCounts.getOrDefault(level, 0) + count);
                }
            }
        }
        
        System.out.println("LevelCountAnalyzer: Counts after merge: " + levelCounts);
    }
}