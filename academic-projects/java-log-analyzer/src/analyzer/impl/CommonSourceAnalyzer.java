/*********************************************************
*
* Description: Log analyzer that identifies the most common log source.
* Author: Moshe Yifrah
*
*********************************************************/

package analyzer.impl;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.TreeMap;

import analyzer.LogAnalyzer;
import model.LogEntry;
import org.json.JSONObject;

/**
 * Analyzes log entries to find the most common and least common sources.
 */
public class CommonSourceAnalyzer implements LogAnalyzer {
    
    private final Map<String, Integer> sourceCounts;
    private String mostCommonSource;
    private int mostCommonSourceCount;
    private String leastCommonSource;
    private int leastCommonSourceCount;
    
    /**
     * Constructor initializing the source counts map.
     */
    public CommonSourceAnalyzer() {
        sourceCounts = Collections.synchronizedMap(new HashMap<>());
        mostCommonSourceCount = -1;
        leastCommonSourceCount = Integer.MAX_VALUE;
    }
    
    @Override
    public String getName() {
        return "FIND_COMMON_SOURCE";
    }
    
    @Override
    public void analyze(List<LogEntry> logEntries, Properties properties) {
        System.out.println("CommonSourceAnalyzer: Analyzing " + logEntries.size() + " entries");
        
        for (LogEntry entry : logEntries) {
            String source = entry.getSource();
            synchronized (sourceCounts) {
                sourceCounts.put(source, sourceCounts.getOrDefault(source, 0) + 1);
            }
        }
        
        // Find most and least common sources
        updateCommonSources();
        
        System.out.println("CommonSourceAnalyzer: Sources after analysis: " + sourceCounts);
        System.out.println("CommonSourceAnalyzer: Most common: " + mostCommonSource + " (" + mostCommonSourceCount + ")");
        System.out.println("CommonSourceAnalyzer: Least common: " + leastCommonSource + " (" + leastCommonSourceCount + ")");
    }
    
    /**
     * Updates the most common and least common sources based on the current counts.
     */
    private void updateCommonSources() {
        synchronized (sourceCounts) {
            for (Map.Entry<String, Integer> entry : sourceCounts.entrySet()) {
                String source = entry.getKey();
                int count = entry.getValue();
                
                if (count > mostCommonSourceCount) {
                    mostCommonSource = source;
                    mostCommonSourceCount = count;
                }
                
                if (count < leastCommonSourceCount) {
                    leastCommonSource = source;
                    leastCommonSourceCount = count;
                }
            }
        }
    }
    
    @Override
    public JSONObject getResults() {
        JSONObject result = new JSONObject();
        
        synchronized (sourceCounts) {
            // Create arrays for sources and counts in the same order
            List<String> sourcesList = new ArrayList<>(sourceCounts.keySet());
            List<Integer> countsList = new ArrayList<>();
            
            for (String source : sourcesList) {
                countsList.add(sourceCounts.get(source));
            }
            
            result.put("sources", sourcesList.toString());
            result.put("source_counts", countsList.toString());
            
            // Handle the case of no sources found
            if (sourcesList.isEmpty()) {
                result.put("most_common_source", "");
                result.put("most_common_source_count", 0);
                result.put("least_common_source", "");
                result.put("least_common_source_count", 0);
            } else {
                result.put("most_common_source", mostCommonSource);
                result.put("most_common_source_count", mostCommonSourceCount);
                result.put("least_common_source", leastCommonSource);
                result.put("least_common_source_count", leastCommonSourceCount);
            }
        }
        
        return result;
    }
    
    @Override
    public void mergeResults(LogAnalyzer other) {
        if (!(other instanceof CommonSourceAnalyzer)) {
            return;
        }
        
        CommonSourceAnalyzer otherAnalyzer = (CommonSourceAnalyzer) other;
        
        synchronized (sourceCounts) {
            synchronized (otherAnalyzer.sourceCounts) {
                for (Map.Entry<String, Integer> entry : otherAnalyzer.sourceCounts.entrySet()) {
                    String source = entry.getKey();
                    int count = entry.getValue();
                    sourceCounts.put(source, sourceCounts.getOrDefault(source, 0) + count);
                }
            }
            
            // Update most common and least common sources after merging
            updateCommonSources();
        }
    }
}