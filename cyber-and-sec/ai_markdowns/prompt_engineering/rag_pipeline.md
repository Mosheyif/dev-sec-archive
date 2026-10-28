# RAG Pipeline Security and Trust Boundaries

## Overview of a RAG Pipeline

A Retrieval-Augmented Generation (RAG) pipeline connects external documents and
user queries to a large language model to generate grounded, context-aware
responses.

## Pipeline Architecture and Trust Boundaries

```text
[ External Sources / Files ]  ──(Untrusted Data 1)──┐
                                                      ▼
[ User Input / Chat UI ]      ──(Untrusted Data 2)──┼──> [ Control 1: Input Validation & Sanitization ]
                                                      │
                                                      ▼
                                           [ Vector Database / Index ]
                                                      │
                                                      ▼
                                           [ Retrieval Engine ]
                                                      │
                                                      ▼
                                           [ Context Window / LLM ]
                                                      │
                                                      ▼
                                           [ Control 2: Output Filtering & Guardrails ]
                                                      │
                                                      ▼
                                           [ Final Trusted Output ]
```

## Untrusted Data Entry Points

- **External Sources and User Files:** Documents, web pages, or files ingested
  into the vector database. These are untrusted because they can harbor indirect
  prompt injections or malicious instructions hidden within the text.
- **User Input:** Direct queries or text entered by the user in the chat
  interface. This is untrusted as it may contain jailbreak attempts or malicious
  commands.

## Security Controls Placement

- **Control 1 (Input Guardrails):** Positioned before data enters the context
  window. It scans user prompts and retrieved documents for prompt injections,
  validates permissions, and enforces access control lists (ACL).
- **Control 2 (Output Guardrails):** Positioned between the LLM and the user
  interface. It filters the generated response to prevent data leakage, exposure
  of sensitive personal information (PII), or unauthorized execution results.
