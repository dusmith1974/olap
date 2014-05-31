unset TMUX
tmux kill-session -t olap
tmux new -d -s olap
tmux switch -t olap
