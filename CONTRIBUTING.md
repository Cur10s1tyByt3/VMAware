# Contribution Guidelines

We have a useful script at `auxiliary/updater.py` that will update:
- the section line numbers in the header banner
- the date of the update
- and the library documentation for technique links

It's highly recommended to use this script before sending the PR so that all the above don't have to be manually updated, which can be time consuming and can potentially creep in some human errors. 

Also, make sure to create your PR merge target to the `dev` branch and not the `main` branch. This is because all our prototype code are developed in `dev`. We usually merge that branch to `main` at least once a week, and it keeps our codebase organised and separated between a prototype that we cautiously don't think it's ready to be used by the public yet (especially if it's a new technique being introduced), and an upstream version that we've deemed to be practically ready.

If you have any questions or inquiries, our contact details are in the README.

Lastly, consider adding your name and github in the vmaware.hpp file and the README's credit sections. Your work is valuable to us, and we want to credit you for the improvements you've made. 

# Q&A
### I want to add a new technique, how would I do that?
There's a few steps that should be taken:
1. Make sure to add the technique name in the enums of all the techniques in the appropriate place.
2. Add the technique function itself in the technique section of the library. Make sure to add it in the right place, as there's preprocessor directives for each platform (Linux, Windows, and Apple)
3. Add the technique in the technique table situated at the end of the header file. The score should be between 10 and 100. Although there are exceptions, it's advised to follow the aforementioned score range.


### I want to make a major change to the library
Depending on how big the change is, if the change is fairly small then just a simple PR is fine. But if it has hundreds of lines of code changes then it's best to create an issue prior to even starting to write the code, or you can discuss it with us, either works.