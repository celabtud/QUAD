# Developer Guidelines
(Please talk to people on the mailing list before you change this page, see our section on [how to get in touch](https://github.com/celabtud/QUAD#get-in-touch))

## Important note for pull requests
**Pull requests should issued against the develop branch**.  We never pull directly into master.

**Our goal is to iterate in small steps. Release often, release early. Evolution instead of a revolution**

## General goals
To make sure everybody is going in the same direction:
* easy to install for admins and easy to use for people
* easy to integrate into other apps, but also usable as standalone
* less memory and execution time requirements
* extensible
* modular

## How to work with git?
* Don't work in your master branch.
* Make a new branch for every feature you're working on. (This ensures that you can work you can do lots of small, independent pull requests instead of one big one with complete different features)
* Don't use the online edit function of github (this only creates ugly and not working commits!)
* Try to make clean commits that are easy readable (including descriptive commit messages!)
* Test before you push. Sounds easy, isn't!
* Don't check in stuff that gets generated during build or runtime
* Make small pull requests that are easy to review but make sure they do add value by themselves / individually

## Coding style
* Do write comments. (You don't have to comment every line, but if you come up with something thats a bit complex/weird, just leave a comment. Bear in mind that you will probably leave the project at some point and that other people will read your code. Undocumented huge amounts of code are worthless!)
* Indentation: 4 spaces
* Don't overengineer. Don't try to solve any possible problem in one step, but try to solve problems as easy as possible and improve the solution over time!
* Do generalize sooner or later! (if an old solution, quickly hacked together, poses more problems than it solves today, refactor it!)
* Keep it compatible.
* If you do make changes, document them! (see below)

## Branching model / git workflow
see [git flow](http://nvie.com/posts/a-successful-git-branching-model)

### `master` branch
* the stable
* This is the branch everyone should use for production stuff

### `develop`branch
* everything that is READY to go into master at some point in time
* This stuff is tested and ready to go out

### `release` branches
* stuff that should go into master very soon
* only bugfixes go into these (see http://nvie.com/posts/a-successful-git-branching-model/ for why)
* we should not be blocking new features to develop, just because we feel that we should be releasing it to master soon. This is the situation that release branches solve/handle.

### hotfix branches
* fixes for bugs in master

### feature branches (in your own repos)
* these are the branches where you develop your features in
* If its ready to go out, it will be merged into develop

Over the time we pull features from feature branches into the develop branch. Every month we pull from develop into master. Bugs in master get fixed in hotfix branches. These branches will get merged into master AND develop. There should never be commits in master that aren't in develop

## Documentation
(Not currently available)
The docs are in the `doc/` folder in the git repository, so people can easily find the suitable docs for the current git revision.

Documentation should be kept up-to-date. This means, whenever you add a new functionality, add necessary documentation in /doc in the same pull request. 
