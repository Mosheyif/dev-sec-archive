# Log Classification Prompt Engineering Analysis

## 1. Zero-Shot Implementation

**Prompt Used:**

> Classify these simple system log file into one of these categories:
> informational, low, medium, high, critical.

**Analysis of Runs:**

- **Consistency:** Low. Across the two zero-shot runs, the model generated
  different table headers (e.g., "Severity Category" vs. "Classification") and
  varied the formatting of its explanations. The order of the logs also appeared
  arbitrary.
- **Label Format:** Poor adherence. The prompt requested lowercase labels, but
  the model capitalized them (e.g., "Informational", "Low") due to standard
  English capitalization rules taking over.
- **Accuracy:** The base classification was technically correct across both
  runs, but the reasoning lacked a standardized professional structure.

---

## 2. Few-Shot Implementation

**Prompt Used:**

> You are a security operations center (SOC) analyst. Analyze and classify raw
> system log files into one of five severity categories: informational, low,
> medium, high, or critical.
> Provide your output as a Markdown table containing the log filename, its
> classification, and a concise justification highlighting key indicators.
> [Included 5 examples with clear delimiters such as `=== log_name.txt ===`]

**Analysis of Runs:**

- **Consistency:** High. The inclusion of examples and output formatting rules
  locked the model into a predictable behavior. Both runs produced identical
  table structures, exact column headers ("Log File", "Classification",
  "Key Indicators & Justification"), and a consistent sorting order.
- **Label Format:** Strict adherence. The model copied the formatting from the
  examples and used exact lowercase labels (e.g., "informational", "critical"),
  making the output suitable for programmatic parsing.
- **Accuracy:** High. The classifications remained accurate, and the
  justifications were highly focused, concise, and aligned with the requested
  SOC analyst persona.

---

## 3. Comparison Conclusion

The zero-shot approach is sufficient for basic categorization but fails when
predictable formatting is required. Upgrading to a few-shot approach with a
defined persona, clear text delimiters, and a strict output specification
eliminates structural variance. This ensures the output can be reliably parsed
by automated systems without manual intervention.
