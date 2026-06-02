---
name: gh-ticketing
description: GitHub CLI ticketing system for solo dev — Feature -> Story -> Task hierarchy via Issues, labels, milestones, and Projects
---

# GitHub CLI Ticketing Skill

Lightweight ticketing system using GitHub Issues + Projects for the `dr-natetorious/unreal_experiments` repo.

## Core Concepts

- **type:feature** — Top-level work item (e.g., "Police Car Vehicle")
- **type:story** — Deliverable slice of a feature (e.g., "Siren Audio System")
- **type:task** — Implementation detail of a story (e.g., "Wire siren sound cue")
- **Hierarchy** — Expressed via GitHub issue body links (`blocks:`, `relates to:`, `Closes #N`)
- **Milestones** — Group related work. One milestone per git feature branch.
- **Labels** — `area:*` prefix for domain tags (audio, physics, meshes, input, animation, ai, ui, blueprint).
- **Projects** — Kanban-style board. Issues/PRs added for visual tracking.

---

## Setup (run once)

```bash
# Ensure project scope
gh auth refresh -s project

# Create labels (use -f to idempotent/update)
gh label create type:feature -d "Epic/Feature" -c 7057ff -f
gh label create type:story -d "User Story" -c 0e8a16 -f
gh label create type:task -d "Sub-task" -c c5def0 -f
gh label create status:blocked -d "Blocked" -c d73a4a -f
gh label create area:audio -d "Sound" -c 5319e7 -f
gh label create area:physics -d "Physics" -c 006b75 -f
gh label create area:meshes -d "3D models" -c d4c5f9 -f
gh label create area:input -d "Controls" -c 008672 -f
gh label create area:animation -d "Animation" -c 0075ca -f
gh label create area:ai -d "AI" -c e4e669 -f
gh label create area:ui -d "HUD" -c f7d120 -f
gh label create area:blueprint -d "Blueprint" -c cfd3d7 -f

# Create project
gh project create --owner "@me" --title "Unreal Experiments Tracker" --format json -q '.number'

# Create milestone
gh api repos/{owner}/{repo}/milestones -f title="Police Vehicle v1" -f state=open
```

---

## Issue Lifecycle

```bash
# Create
gh issue create -t "FEAT-001 Police Car Vehicle" --label type:feature --milestone "Police Vehicle v1" -b "Full police car with driving, lights, siren."

# Multiple labels
gh issue create -t "TASK-003 Wire siren" --label type:task --label area:audio -b $'**Parent:** #2 (STOR-001)'

# List
gh issue list                                    # all open
gh issue list --label type:task                  # by label
gh issue list --state all                        # all
gh issue list --milestone "Police Vehicle v1"    # by milestone
gh issue list --json number,title,labels         # JSON output
gh issue list --jq '.[] | "#\(.number) \(.title)"'  # with jq filter

# View
gh issue view 1
gh issue view 1 -c            # with comments
gh issue view 1 --json title,body,labels

# Edit
gh issue edit 1 -t "New title"
gh issue edit 1 --add-label area:audio --remove-label area:ui
gh issue edit 1 --add-assignee "@me"
gh issue edit 1 -b $'**Parent:** #1\n\nUpdated body...'

# Comment
gh issue comment 1 -b "Update: blocked on engine."

# Close/Reopen/Delete
gh issue close 1
gh issue reopen 1
gh issue delete 1
```

## Milestones (via API)

```bash
# Create
gh api repos/{owner}/{repo}/milestones -f title="Milestone Name" -f state=open

# List
gh api repos/{owner}/{repo}/milestones?state=open

# Close
gh api -X PATCH repos/{owner}/{repo}/milestones/123 -f state=closed

# Delete (must be closed first)
gh api -X DELETE repos/{owner}/{repo}/milestones/123
```

## Linking Issues (Hierarchy)

GitHub doesn't have parent/child. Use body text + keywords:

```markdown
## Related
- **Parent Feature:** #1 (FEAT-001 Police Car Vehicle)
- **Parent Story:** #5 (STOR-002 Siren Audio System)
```

GitHub auto-detects these keywords in body/PR description:
- `blocks #N`, `blocked by #N`, `relates to #N`, `duplicates #N`, `supersedes #N`
- `Closes #N`, `Fixes #N` → auto-closes on PR merge

## Projects

```bash
# Create (one-time)
gh project create --owner "@me" --title "Unreal Experiments Tracker" --format json -q '.number'

# View
gh project view 3 --owner "@me"

# Add issue to project
gh project item-add 3 --owner "@me" --url https://github.com/dr-natetorious/unreal_experiments/issues/1

# Create issue and add to project in one go
gh issue create --title "..." --project "Unreal Experiments Tracker"

# List items
gh project item-list 3 --owner "@me" --format json
gh project item-list 3 --owner "@me" --query "assignee:@me is:issue is:open"

# Create a status field
gh project field-create 3 --owner "@me" \
  --name "Status" --data-type "SINGLE_SELECT" \
  --single-select-options "Todo,In Progress,Done,Blocked"
```

## Pull Requests

```bash
# Create with autofill from commits
gh pr create --fill

# Create explicitly, auto-closing issue
gh pr create \
  -t "feat: wire siren toggle" \
  -b $'Closes #5\n\n## Changes\n- Wired ToggleLights event' \
  --label area:audio

# List/View/Checkout
gh pr list
gh pr view 1 -c
gh pr checkout 1
gh pr merge 1 --delete-branch
```

## Search

```bash
gh search issues "police car"
gh search issues --label type:task --state open
gh search issues --assignee @me --state open
```

## Formatting

| Flag | Use | Example |
|------|-----|---------|
| `--json fields` | JSON output | `--json number,title,labels` |
| `--jq expr` | Filter JSON | `--jq '.[].number'` |
| `--template tmpl` | Go templates | `--template '{{range .}}{{.title}}{{"\n"}}{{end}}'` |

## Gotchas

1. **Project scope** — `gh project` needs `project` token scope. If 404, run `gh auth refresh -s project`.
2. **Shell quoting** — use `$'...\n...'` for multiline body in bash.
3. **Search hyphens** — exclusion queries need `--`: `gh search issues -- "-label:bug"`.
4. **`gh api` placeholders** — `{owner}`, `{repo}`, `{branch}` auto-replace from git context.
5. **Label colons** — quote if shell is sensitive: `"type:feature"`.
