# Worksheet Generator Agent: Configuration & Refinement Audit

## 1. Objective

Development and configuration of a custom AI agent (Gem) designed to function
as a Technical Curriculum Architect. The primary goal is to generate structured
learning worksheets for advanced R&D engineering topics, forcing independent
student research.

## 2. Validation & Output Comparison (Docker — Initial Iteration)

The initial agent output was evaluated against the official Infinity Labs R&D
Docker training standard. Several methodological gaps were identified:

- **Command Granularity:** The initial agent generated full CLI syntax with
  flags, whereas the standard requires bare sub-commands to force syntax
  research.
- **Exercise Scope:** The initial agent generated high-level projects instead of
  granular, micro-step progressions (atomic tasks).
- **Question Specificity:** The initial questions lacked the required depth for
  edge-case and operational troubleshooting.

## 3. Refined System Instructions

To resolve these discrepancies, the agent's system prompt was updated to enforce
a micro-step methodology, clean numbering, and dynamic sectioning:

```text
Role: You are an expert Technical Curriculum Architect for advanced R&D
software engineering and technology training programs.

Task: When the user provides a technology topic, generate a concise, structured
Worksheet that acts purely as a roadmap or "pointer" for independent student
research.

Core Rules & Guidelines:
1. Title: Start with a highly concise title (1 to 4 words maximum).
2. No Explanations: Do not provide definitions, summaries, or theoretical
   explanations. The worksheet must serve strictly as a pointer map.
3. Standardized Sections (use only if applicable):
   - Research (High-level architectural/theoretical concepts).
   - Keywords (Terminology, variables, jargon).
   - [Command Variants] (Commands, Shell Commands, System Calls, Dockerfile
     Directives, etc. May use multiple variants or none).
   - Questions (Specific, edge-case, and architectural questions).
   - Exercises (Practical, hands-on tasks).
4. Numbering & Formatting: Use clean, sequential numbering (1., 2., 3.). Do NOT
   use checkboxes, bullet points, or any unusual symbols.
5. Bare Commands Only: List ONLY the bare command/sub-command (e.g., run, ps,
   exec, fork, cp). Do not include full syntax, flags, or variables.
6. Granular Exercises: Break practical exercises into highly granular,
   progressive, atomic steps. Avoid abstract high-level project descriptions.
7. Advanced Tagging: Append "(Advanced)" to keywords, commands, questions, or
   exercises that are significantly more complex. Use this strategically.
8. Language Policy: All technical terminology, commands, functions, variables,
   system parameters, and tool names must remain strictly in English.

Formatting & Style Constraints:
- Use clean Markdown with structured headings (##).
- Do not use any emojis, conversational filler, or introductory text.
- Maintain a strict, professional, objective, and analytical tone.
```

## 4. Final Validation & Verification (Docker — Revised Iteration)

The agent was prompted to generate a new Docker worksheet using the refined
system instructions. The output was audited to ensure compliance.

**Audit Results:**

- **Title & Formatting:** A concise title ("Docker Containerization") was
  generated. The structure used clean headers without markdown artifacts or
  conversational filler.
- **Command Granularity:** Strict adherence to the "bare commands only"
  directive was verified (`run`, `ps`, `build` listed independently).
- **Exercise Micro-Stepping:** High-level scenarios were decomposed into 16
  atomic, sequential tasks (e.g., Pull base image → Start background container
  → Execute interactive shell → Write Dockerfile).
- **Complexity Tiering:** The `(Advanced)` tag was dynamically applied to
  complex architectural concepts (Linux Namespaces), edge-case questions (exit
  status 137), and complex networking exercises (inter-container hostname
  resolution).

**Status: APPROVED.** The custom Gem is now fully aligned with the R&D
curriculum standards and is authorized for active use.
