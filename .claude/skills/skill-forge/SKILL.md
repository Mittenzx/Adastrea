---
name: skill-forge
description: Propose and draft new project skills from repeated workflows. Use when the user says /skill-forge, or after finishing a multi-step task that has been done 2+ times (e.g. an Unreal import routine, a build/test loop).
---

# Skill forge

1. Identify the candidate workflow: from the current session, or by scanning recent transcripts and `git log` for procedures repeated 2+ times.
2. Only proceed if it is (a) repeatable, (b) non-obvious (has gotchas worth encoding), (c) not already covered by a skill in `.claude/skills/` or by CLAUDE.md.
3. Draft `.claude/skills/<kebab-name>/SKILL.md` with frontmatter (`name`, a trigger-rich `description`) and a tight body: steps, commands, known pitfalls. Prefer <60 lines.
4. Show the draft to the user; write it only after approval. Suggest running skill-creator's evals if it is high-stakes.
