# Tokenization Exercise Analysis

## Tool and Model Used

- **Tokenizer Tool:** [OpenAI Tokenizer](https://platform.openai.com/tokenizer)
- **Model Type:** GPT-5.x & O1/3

---

## Exercise Steps and Results

### 1. English Sentence

- **Input Text:** `The quick brown fox jumps over the lazy dog.`
- **Character Count:** 44 characters
- **Token Count:** 10 tokens

### 2. 64-Character Hash

- **Input Text:**
  `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`
- **Character Count:** 64 characters
- **Token Count:** 35 tokens

### 3. Short Code Snippet

- **Input Text:**

  ```c
  int main() {
      return 0;
  }
  ```

- **Character Count:** 28 characters
- **Token Count:** 10 tokens

### 4. Hebrew Sentence

- **Input Text:** `המהיר והחום קפץ מעל הכלב העצלן.`
- **Character Count:** 31 characters
- **Token Count:** 16 tokens

---

## Analysis of Token Efficiency and Impacts

### Which Inputs Are "Token-Expensive"?

- **64-Character Hash** and **Hebrew sentence** are considered "token-expensive"
  relative to their character length.
- Random or non-standard character sequences (like cryptographic hashes) break
  down into tiny fragments because they do not match common linguistic patterns
  in the tokenizer's dictionary, resulting in a very high token-to-character
  ratio. Similarly, non-English languages like Hebrew require more tokens per
  character compared to English text.

### Why Might That Matter for Cost?

- Large language model API providers bill users based on the total number of
  tokens processed (both input prompts and output generations).
- Using token-expensive inputs means you consume more tokens for the exact same
  amount of raw data or text length, leading to significantly higher API usage
  costs.

### Why Might That Matter for the Context Window?

- LLMs have a strict, hard limit on their context window (measured in maximum
  allowable tokens).
- Token-expensive inputs consume the available memory space much faster. Feeding
  the model data with poor token efficiency (such as binary files, hashes, or
  non-English text) will saturate the context window prematurely, causing the
  model to hit its capacity limits or forget preceding conversation history much
  sooner.
