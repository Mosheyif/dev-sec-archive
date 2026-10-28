/*********************************************************
*
* Description: Parallel log processing coordinator — reads files and dispatches analyzers.
* Author: Moshe Yifrah
*
*********************************************************/

package logprocessor;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.stream.Collectors;

import analyzer.LogAnalyzer;
import analyzer.factory.LogAnalyzerFactory;
import org.json.JSONObject;

/**
 * Main class for processing log files in parallel and generating a report.
 */
public class LogProcessor {
    
    private final Properties config;
    private final LogFileReader fileReader;
    private final Map<String, LogAnalyzer> globalAnalyzers;
    
    /**
     * Constructor for creating a log processor.
     * 
     * @param config Configuration properties.
     */
    public LogProcessor(Properties config) {
        this.config = config;
        this.fileReader = new LogFileReader();
        this.globalAnalyzers = new HashMap<>();
    }
    
    /**
     * Processes log files in parallel and generates a report.
     */
    public void processLogs() {
        try {
            // Get the log directory
            String logDirectory = config.getProperty("log.directory");
            List<File> logFiles = fileReader.getLogFiles(logDirectory);
            
            if (logFiles.isEmpty()) {
                System.out.println("No log files found in directory: " + logDirectory);
                return;
            }
            
            // Get the thread pool size
            int threadPoolSize = Integer.parseInt(config.getProperty("thread.pool.size", "5"));
            
            // Get the analysis types to perform
            String analysisTypesStr = config.getProperty("log.analysis", "COUNT_LEVELS");
            List<String> analysisTypes = Arrays.asList(analysisTypesStr.split(","))
                                               .stream()
                                               .map(String::trim)
                                               .collect(Collectors.toList());
            
            // Create analyzers for each analysis type
            for (String type : analysisTypes) {
                LogAnalyzer analyzer = LogAnalyzerFactory.createAnalyzer(type);
                if (analyzer != null) {
                    globalAnalyzers.put(type, analyzer);
                }
            }
            
            if (globalAnalyzers.isEmpty()) {
                System.out.println("No valid analysis types specified.");
                return;
            }
            
            // Process the log files in parallel
            processFilesInParallel(logFiles, threadPoolSize);
            
            // Generate and save the report
            String outputFile = config.getProperty("output.file", "output.json");
            generateReport(outputFile);
            
        } catch (Exception e) {
            System.out.println("Error processing logs: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    /**
     * Processes log files in parallel using a thread pool.
     * 
     * @param logFiles The log files to process.
     * @param threadPoolSize The size of the thread pool.
     * @throws InterruptedException If the thread execution is interrupted.
     * @throws ExecutionException If an error occurs during execution.
     */
    private void processFilesInParallel(List<File> logFiles, int threadPoolSize) 
            throws InterruptedException, ExecutionException {
        
        // Create a fixed thread pool
        ExecutorService executor = Executors.newFixedThreadPool(threadPoolSize);
        
        // Create a list to hold the Future results
        List<Future<Map<String, LogAnalyzer>>> futures = new ArrayList<>();
        
        // Submit tasks for each log file
        for (File logFile : logFiles) {
            // Create a new set of analyzers for this file
            Map<String, LogAnalyzer> fileAnalyzers = new HashMap<>();
            for (String type : globalAnalyzers.keySet()) {
                fileAnalyzers.put(type, LogAnalyzerFactory.createAnalyzer(type));
            }
            
            // Create and submit a worker for this file
            LogProcessingWorker worker = new LogProcessingWorker(logFile, fileAnalyzers, config, fileReader);
            futures.add(executor.submit(worker));
        }
        
        // Process the results as they complete
        for (Future<Map<String, LogAnalyzer>> future : futures) {
            try {
                Map<String, LogAnalyzer> fileAnalyzers = future.get();
                
                // Merge the results with the global analyzers
                for (Map.Entry<String, LogAnalyzer> entry : fileAnalyzers.entrySet()) {
                    String type = entry.getKey();
                    LogAnalyzer fileAnalyzer = entry.getValue();
                    LogAnalyzer globalAnalyzer = globalAnalyzers.get(type);
                    
                    if (globalAnalyzer != null) {
                        globalAnalyzer.mergeResults(fileAnalyzer);
                    }
                }
            } catch (Exception e) {
                System.out.println("Error processing file: " + e.getMessage());
            }
        }
        
        // Shutdown the executor
        executor.shutdown();
    }
    
    /**
     * Generates a JSON report with the analysis results and saves it to a file.
     * 
     * @param outputFile The file to save the report to.
     */
    private void generateReport(String outputFile) {
        JSONObject report = new JSONObject();
        
        // Add the results of each analyzer to the report
        for (LogAnalyzer analyzer : globalAnalyzers.values()) {
            String analyzerName = analyzer.getName();
            JSONObject results = analyzer.getResults();
            
            if (analyzerName.equals("DETECT_ANOMALIES")) {
                // Special handling for anomalies as per requirements
                report.put(analyzerName, results.get(analyzerName));
            } else {
                report.put(analyzerName, results);
            }
        }
        
        // Save the report to a file
        try (FileWriter writer = new FileWriter(outputFile)) {
            writer.write(report.toString(2)); // Pretty print with 2-space indentation
            System.out.println("Report saved to " + outputFile);
        } catch (IOException e) {
            System.out.println("report saving error");
        }
    }
}