# Godot 4
Documenting some godot 4 notes that I might forget for future me to have a chance at running/updating anything in this folder a month after working on it.

## PR Process Notes
* Resync forked branch up with master
  * Use github gui (hit the green "Sync Fork" button)
  * On PR branch:
    * `git pull --rebase origin master`
    * `git push -f origin {pr branch name}`
* Adding additional changes after initial PR commit.
  * Godot wants a clean "1 commit" per PR history, so need to amend any changes to the original commit.
    * `git commit --amend`
    * `git push -f origin {pr branch name}`

## Build Notes
* Uses scons. Most logging is disabled unless "dev_build" is set.
  * `scons target=editor dev_build=yes`
