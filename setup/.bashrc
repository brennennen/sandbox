#
# ~/.bashrc
#

# If not running interactively, don't do anything
[[ $- != *i* ]] && return

alias ls='ls --color=auto'
alias grep='grep --color=auto'
PS1='[\u@\h \W]\$ '

# Sane history defaults
export HISTCONTROL=erasedups:ignorespace
shopt -s histappend

export HISTSIZE=10000
export HISTFILESIZE=10000

# Search history based on what's already typed
bind '"\e[A": history-search-backward'
bind '"\e[B": history-search-forward'

# Aliases
alias ll='ls -alFh'
alias la='ls -A'
alias sl='ls'
alias df='df -h'
alias du='du -h'
alias tmux0='tmux a -t 0'

# Defaults
export EDITOR='nvim'

# Bash autocomplete
if [ -f /etc/bash_completion ]; then
	. /etc/bash_completion
fi

# Useful user level package manager (when not allowed sudo access)
export PATH="/home/b/.pixi/bin:$PATH"

# fuzzy finder
[ -f /usr/share/fzf/key-bindings.bash ] && source /usr/share/fzf/key-bindings.bash
