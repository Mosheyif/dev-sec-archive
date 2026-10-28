/*********************************************************
*
* Description: Strategy interface for log analysis algorithms.
* Author: Moshe Yifrah
*
*********************************************************/

package analyzer;

import java.util.List;
import java.util.Properties;

import model.LogEntry;
import org.json.JSONObject;

/**
 * Interface for all log analyzers.
 * Uses the Strategy design pattern to allow different analysis implementations.
 */
public interface LogAnalyzer {
    
    /**
     * Gets the name of the analyzer, used for identifying it in configuration and output.
     * 
     * @return The name of the analyzer.
     */
    String getName();
    
    /**
     * Processes a list of log entries and performs the analysis.
     * 
     * @param logEntries The log entries to analyze.
     * @param properties Configuration properties.
     */
    void analyze(List<LogEntry> logEntries, Properties properties);
    
    /**
     * Returns the analysis results as a JSON object.
     * 
     * @return A JSONObject containing the analysis results.
     */
    JSONObject getResults();
    
    /**
     * Merges the results of this analyzer with results from another instance of the same analyzer.
     * This is used when combining results from parallel processing.
     * 
     * @param other Another instance of the same analyzer.
     */
    void mergeResults(LogAnalyzer other);
}