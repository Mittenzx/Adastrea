# Unreal + Blender experts for Claude Code: setup

Do this on the laptop, in the folder that contains your `.uproject`.

## 1. Install the two skills

Unzip this kit into your project folder. You should end up with:

```
YourProject/
  YourProject.uproject
  .claude/skills/unreal-editor-expert/SKILL.md
  .claude/skills/blender-expert/SKILL.md
  CLAUDE_SETUP.md   <- this file, delete it when you're done
```

To have the experts in every project instead, move the two skill folders to `C:\Users\<you>\.claude\skills\`.

Open a terminal in the project folder and start Claude Code with `claude`.

If your `.blend` files live outside the project folder, run `/add-dir "D:\path\to\blender\files"` inside Claude Code so it can see them.

## 2. Teach it your project

Paste this into Claude Code:

> Use the unreal-editor-expert and blender-expert skills to get to know this project. Don't change anything except creating CLAUDE.md.
> 1. Read the .uproject, Config/*.ini, any Build.cs files and Source/. Note the engine version, C++ or Blueprint-only, enabled plugins, default maps, game mode and input setup.
> 2. List the Content/ folder structure (names only). If the Unreal engine is installed and the editor is closed, run the unreal-editor-expert list_assets.py script through the commandlet to get asset types.
> 3. Find the Blender source files. Run the blender-expert inspect_blend.py script on the main ones and note units, scale or naming problems, and where exports go.
> 4. Write CLAUDE.md in the project root: a short overview, versions, a folder map, key maps and assets, naming conventions, the Blender to Unreal export workflow, and a list of open questions for me. Keep it under about 150 lines.

Read the CLAUDE.md it writes and correct anything wrong. Claude Code loads it at the start of every session, so it's worth getting right.

## 3. Optional: let Claude drive the editors live

These give Claude direct control of the running apps. They run code inside Unreal and Blender, so save and commit first, and don't use them with permission prompts turned off.

**Unreal 5.8 or newer (Epic's official plugin):**
1. In Claude Code: `/plugin install unreal-engine-skills-for-claude-code@claude-plugins-official`
2. In Unreal: enable the Unreal MCP (ModelContextProtocol) and AllToolsets plugins, then restart the editor.
3. In the Unreal console: `ModelContextProtocol.StartServer`, then `ModelContextProtocol.GenerateClientConfig ClaudeCode` to write the connection config.
4. On Windows, the plugin's startup hook needs Git for Windows (Git Bash). The tools still work without it.
5. Restart Claude Code, run `/mcp` to check the connection, then try "List all actors in the current level".

**Older Unreal versions:** community servers exist, such as UEMCP (uses Unreal's built-in Python remote execution; its docs list UE 5.0 to 5.6). Check its README before installing.

**Blender 5.1 or newer:** the official Blender Lab MCP server (blender.org/lab/mcp-server) needs its add-on plus the server; follow its setup wiki. Its own docs warn it runs AI-generated code with no guards.

**Older Blender versions:** the community blender-mcp project (github.com/ahujasid/blender-mcp) has an add-on plus a server. Its README gives the Claude Code command (it has been `claude mcp add blender uvx blender-mcp`, which needs `uv`); check it before installing.

## 4. Use it from your phone

1. On the laptop, start Claude Code with Remote Control: `claude --rc "UE project"` (or type `/rc` in a session that's already running).
2. To make every session reachable, run `/config` and turn on Enable Remote Control for all sessions.
3. On your phone, open the Claude app, tap Code, and pick the session (green dot means online).

The laptop has to stay on with Claude Code running; the phone is just a window into that session. Remote Control needs you signed in with your Claude subscription, not an API key.
