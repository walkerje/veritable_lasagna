## Coding standards

- Prefer clear, readable C over clever C.
  - Unless the nature of the problem demands otherwise. If you think so, explain why.
- Keep functions small and focused.
- Avoid unnecessary global state.
  - **All global state must be either constant, thread-local, or thread-safe in practice.**
- Add comments for *why* something is done, not what the code already says.
- Code should be formatted with `clang-format`.

## Commit messages

- Use imperative mood: “Fix crash on empty input”
- Keep the first line under ~72 characters when possible.
- Reference issues when relevant: `Fixes #123`

## Pull request checklist

- [ ] I have read the [Code of Conduct](CODE_OF_CONDUCT.md) and agree to abide by its terms.
- [ ] I formatted the code with `clang-format`
- [ ] I built the **library and tests** with strict compilation enabled (**`VL_STRICT_BUILD` / `-DVL_STRICT_BUILD=ON`** | see README for details)
- If I added, changed, or removed any compiler-specific or platform-specific code:
  - [ ] I verified the build compiles under all of:
      - [ ] MSVC SDK 10.0.26100+
      - [ ] GCC 13.3.0+
      - [ ] Clang 18.0.0+
- [ ] I ran relevant tests (or explained why not)
- [ ] I added/updated tests where appropriate
- [ ] I added/updated documentation where appropriate
- [ ] If my change was toolchain-specific, I added a note in the pull request description.

## Reporting issues / requesting features

When filing an issue, please include:
- Expected behavior vs. actual behavior
- Environment (OS, compiler version, CMake version, etc.)
- Minimal reproducer or test case, when applicable

When requesting a feature, please include:
- Use case
- How it would be used
- Why it belongs in the library relative to other features and/or available libraries

## Community expectations

Be kind, be constructive, assume good intent, and enjoy what you do. 🚀

Feel free to ask questions, provide feedback, and contribute to the project! 🌟