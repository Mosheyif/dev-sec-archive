/*********************************************************
*
* Description: Worker task for processing a single log file in a thread pool.
* Author: Moshe Yifrah
*
*********************************************************/

package logprocessor;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.Callable;

import analyzer.LogAnalyzer;
import model.LogEntry;

/**
 * Worker class that processes a log file in a separate thread.
 */
public class LogProcessingWorker implements Callable<Map<String, LogAnalyzer>> {

    private final File logFile;
    private final Map<String, LogAnalyzer> analyzers;
    private final Properties config;
    private final LogFileReader fileReader;
    
    public LogProcessingWorker(File logFile, Map<String, LogAnalyzer> analyzers, 
                              Properties config, LogFileReader fileReader) {
        this.logFile = logFile;
        this.analyzers = analyzers;
        this.config = config;
        this.fileReader = fileReader;
    }
    
    @Override
    public Map<String, LogAnalyzer> call() throws Exception {
        System.out.println("Worker starting for file: " + logFile.getName());
        try {
            // Read the log file
            List<LogEntry> entries = fileReader.readLogFile(logFile);
            
            // Apply each analyzer to the entries
            for (Map.Entry<String, LogAnalyzer> entry : analyzers.entrySet()) {
                String analyzerName = entry.getKey();
                LogAnalyzer analyzer = entry.getValue();
                
                System.out.println("Applying analyzer " + analyzerName + " to " + logFile.getName());
                analyzer.analyze(entries, config);
                System.out.println("Analysis complete for " + analyzerName);
            }
            
            System.out.println("Worker completed for file: " + logFile.getName());
            return analyzers;
        } catch (IOException e) {
            System.out.println("error processing file " + logFile.getName() + ": " + e.getMessage());
            throw e;
        }
    }
}