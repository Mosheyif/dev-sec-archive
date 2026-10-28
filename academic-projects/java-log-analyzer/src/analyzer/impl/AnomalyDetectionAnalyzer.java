/*********************************************************
*
* Description: Log analyzer that detects error spikes using a sliding time window.
* Author: Moshe Yifrah
*
*********************************************************/

package analyzer.impl;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import java.util.TreeSet;
import java.util.stream.Collectors;

import analyzer.LogAnalyzer;
import model.FileAwareLogEntry;
import model.LogEntry;
import org.json.JSONArray;
import org.json.JSONObject;

/**
 * Analyzes log entries to detect anomalies based on the frequency of certain log levels
 * within a specified time window.
 */
public class AnomalyDetectionAnalyzer implements LogAnalyzer {
    
    private final Map<String, Set<String>> fileAnomalies;
    private List<String> targetLevels;
    private int timeWindowSeconds;
    private int threshold;
    
    /**
     * Constructor initializing the file anomalies map.
     */
    public AnomalyDetectionAnalyzer() {
        fileAnomalies = Collections.synchronizedMap(new HashMap<>());
    }
    
    @Override
    public String getName() {
        return "DETECT_ANOMALIES";
    }
    
    @Override
    public void analyze(List<LogEntry> logEntries, Properties properties) {
        if (logEntries.isEmpty()) {
            System.out.println("AnomalyDetectionAnalyzer: No entries to analyze");
            return;
        }
        
        System.out.println("AnomalyDetectionAnalyzer: Starting analysis of " + logEntries.size() + " entries");
        
        // Parse configuration
        String levelsStr = properties.getProperty("log.analysis.anomalies.levels", "ERROR");
        targetLevels = Arrays.stream(levelsStr.split(","))
                              .map(String::trim)
                              .collect(Collectors.toList());
        
        timeWindowSeconds = Integer.parseInt(properties.getProperty("log.analysis.anomalies.window", "30"));
        threshold = Integer.parseInt(properties.getProperty("log.analysis.anomalies.threshold", "2"));
        
        System.out.println("AnomalyDetectionAnalyzer: Config - levels=" + targetLevels + 
                           ", window=" + timeWindowSeconds + "s, threshold=" + threshold);
        
        // Group entries by file
        Map<String, List<LogEntry>> entriesByFile = groupEntriesByFile(logEntries);
        
        // Process each file's entries
        for (Map.Entry<String, List<LogEntry>> fileEntries : entriesByFile.entrySet()) {
            String fileName = fileEntries.getKey();
            List<LogEntry> entries = fileEntries.getValue();
            
            // Filter and sort entries for this file
            List<LogEntry> targetEntries = entries.stream()
                                                .filter(e -> targetLevels.contains(e.getLevel()))
                                                .sorted((e1, e2) -> e1.getTimestamp().compareTo(e2.getTimestamp()))
                                                .collect(Collectors.toList());
            
            System.out.println("AnomalyDetectionAnalyzer: Found " + targetEntries.size() + 
                              " entries matching target levels: " + targetLevels + " in file: " + fileName);
            
            // Detect anomalies for this file
            if (!targetEntries.isEmpty()) {
                detectSimpleAnomalies(fileName, targetEntries);
            }
        }
        
        System.out.println("AnomalyDetectionAnalyzer: Analysis complete");
    }
    
    /**
     * Groups log entries by the file they came from.
     * 
     * @param entries List of log entries
     * @return Map of file names to lists of entries from that file
     */
    private Map<String, List<LogEntry>> groupEntriesByFile(List<LogEntry> entries) {
        Map<String, List<LogEntry>> entriesByFile = new HashMap<>();
        
        for (LogEntry entry : entries) {
            String fileName;
            
            if (entry instanceof FileAwareLogEntry) {
                fileName = ((FileAwareLogEntry) entry).getFileName();
            } else {
                // Fallback if entries aren't FileAwareLogEntry instances
                fileName = "unknown.log";
            }
            
            // Add entry to the appropriate file group
            entriesByFile.computeIfAbsent(fileName, k -> new ArrayList<>()).add(entry);
        }
        
        return entriesByFile;
    }
    
    /**
     * Detects anomalies using a simple counting approach within time windows.
     * 
     * @param fileName Name of the log file being analyzed
     * @param entries Log entries to analyze
     */
    private void detectSimpleAnomalies(String fileName, List<LogEntry> entries) {
        if (entries.isEmpty()) {
            System.out.println("No entries to analyze for anomalies in file: " + fileName);
            return;
        }
        
        // Create a set for anomaly timestamps for this file
        Set<String> anomalyTimes = Collections.synchronizedSet(
            fileAnomalies.computeIfAbsent(fileName, k -> new TreeSet<>())
        );
        
        // Simple approach: just count consecutive entries within the time window
        for (int i = 0; i < entries.size(); i++) {
            LogEntry current = entries.get(i);
            LocalDateTime windowEnd = current.getTimestamp().plusSeconds(timeWindowSeconds);
            
            int count = 1; // Count the current entry
            
            // Count entries within the time window from the current entry
            for (int j = i + 1; j < entries.size() && 
                 !entries.get(j).getTimestamp().isAfter(windowEnd); j++) {
                count++;
            }
            
            // If count exceeds threshold, record an anomaly
            if (count >= threshold) {
                System.out.println("Found anomaly in file " + fileName + " at " + current.getFormattedTimestamp() + 
                                   " with " + count + " entries in " + timeWindowSeconds + "s window");
                anomalyTimes.add(current.getFormattedTimestamp());
                
                // Move past this window to avoid overlapping anomalies
                while (i < entries.size() && 
                       !entries.get(i).getTimestamp().isAfter(windowEnd)) {
                    i++;
                }
                i--; // Adjust for the loop increment
            }
        }
        
        System.out.println("Total anomalies detected for " + fileName + ": " + anomalyTimes.size());
    }
    
    @Override
    public JSONObject getResults() {
        System.out.println("AnomalyDetectionAnalyzer: Getting results");
        JSONArray anomaliesArray = new JSONArray();
        
        synchronized (fileAnomalies) {
            for (Map.Entry<String, Set<String>> entry : fileAnomalies.entrySet()) {
                String fileName = entry.getKey();
                Set<String> anomalyTimes = entry.getValue();
                
                if (!anomalyTimes.isEmpty()) {
                    JSONObject fileObj = new JSONObject();
                    JSONObject anomalyObj = new JSONObject();
                    
                    JSONArray anomaliesJsonArray = new JSONArray();
                    for (String time : anomalyTimes) {
                        anomaliesJsonArray.put(time);
                    }
                    
                    anomalyObj.put("anomalies", anomaliesJsonArray);
                    anomalyObj.put("anomalies_count", anomalyTimes.size());
                    
                    fileObj.put(fileName, anomalyObj);
                    anomaliesArray.put(fileObj);
                }
            }
        }
        
        return new JSONObject().put(getName(), anomaliesArray.length() > 0 ? anomaliesArray : new JSONArray());
    }
    
    @Override
    public void mergeResults(LogAnalyzer other) {
        if (!(other instanceof AnomalyDetectionAnalyzer)) {
            return;
        }
        
        AnomalyDetectionAnalyzer otherAnalyzer = (AnomalyDetectionAnalyzer) other;
        
        synchronized (fileAnomalies) {
            synchronized (otherAnalyzer.fileAnomalies) {
                for (Map.Entry<String, Set<String>> entry : otherAnalyzer.fileAnomalies.entrySet()) {
                    String fileName = entry.getKey();
                    Set<String> anomalyTimes = entry.getValue();
                    
                    Set<String> existingAnomalies = fileAnomalies.computeIfAbsent(fileName, 
                                                      k -> Collections.synchronizedSet(new HashSet<>()));
                    
                    synchronized (existingAnomalies) {
                        synchronized (anomalyTimes) {
                            existingAnomalies.addAll(anomalyTimes);
                        }
                    }
                }
            }
        }
    }
}