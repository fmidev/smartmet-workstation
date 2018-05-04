# gitflow
Scripts to make branching and maintaining repositories easier.

## Setting Up

Make this repositorio a [subrepo](https://github.com/ingydotnet/git-subrepo)/subfolder somewhere inside your repo.

Add a scripts folder to the root of your project.

Add the script below to the scripts folder:
```
  #!/usr/bin/env bash

  mkdir -p git
  cp -r ../libs/gitflow/* ./git/
  rm  ./git/LICENSE
  rm ./git/README.md
```
add scripts/git folder to .gitignore but do not ignore master_branch or dev_branch:

```
scripts/git/
!scripts/git/branches/dev_branch
!scripts/git/branches/master_branch
```

Run above script. It should populate the scripts folder with git/

Decide a name for your master and dev branches and write those to branches/dev_branch and branches/master_branch. For example 'dev' and 'master' would be a good idea.

If you made any changes to files that were copied from gitflow copy modified files back to gitflow folder and overwrite.

Commit changes to your repository.

## Usage

There is two kinds of flow going on:

### Dev-Master flow
This is the flow for making new releases. Completing the flow cycle is done by double-clicking or running from commandline the following scripts:
```
dev_1_merge_dev_to_master.sh
dev_2_push_master.sh
```

### Dev-Task flow
This is the usual developer flow. You choose a task from the backlog and run: 
```
task_1_starting_task.sh
```
This script prompts a name for the task branch to be created and checkouts it. Complete the task. Make the usual commits. Complete the the cycle with the following scripts:
```
task_2_merge_task_to_dev.sh
task_3_delete_task.sh
task_4_push_dev.sh
```
Pay attention to the output.
