/*********************************************************
*
* Description: Factory for creating LogAnalyzer instances by type name.
* Author: Moshe Yifrah
*
*********************************************************/

package analyzer.factory;

import java.util.HashMap;
import java.util.Map;
import java.util.function.Supplier;

import analyzer.LogAnalyzer;
import analyzer.impl.AnomalyDetectionAnalyzer;
import analyzer.impl.CommonSourceAnalyzer;
import analyzer.impl.LevelCountAnalyzer;

/**
 * Factory for creating LogAnalyzer instances.
 * Uses the Factory design pattern to create different analyzer types.
 */
public class LogAnalyzerFactory {
    
    private static final Map<String, Supplier<LogAnalyzer>> ANALYZERS = new HashMap<>();
    
    static {
        // Register available analyzers
        registerAnalyzer("COUNT_LEVELS", LevelCountAnalyzer::new);
        registerAnalyzer("FIND_COMMON_SOURCE", CommonSourceAnalyzer::new);
        registerAnalyzer("DETECT_ANOMALIES", AnomalyDetectionAnalyzer::new);
    }
    
    /**
     * Registers a new analyzer type.
     * 
     * @param name The name of the analyzer.
     * @param supplier A supplier that creates instances of the analyzer.
     */
    public static void registerAnalyzer(String name, Supplier<LogAnalyzer> supplier) {
        ANALYZERS.put(name, supplier);
    }
    
    /**
     * Creates an analyzer of the specified type.
     * 
     * @param type The type of analyzer to create.
     * @return A new instance of the requested analyzer, or null if the type is unknown.
     */
    public static LogAnalyzer createAnalyzer(String type) {
        Supplier<LogAnalyzer> supplier = ANALYZERS.get(type);
        
        if (supplier == null) {
            System.out.println(type + " is unknown");
            return null;
        }
        
        return supplier.get();
    }
}