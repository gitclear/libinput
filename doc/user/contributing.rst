
.. _contributing:

==============================================================================
Contributing to libinput
==============================================================================


So you want to contribute to libinput? Great! We'd love to help you be a part
of our community. Here is some important information to help you.

.. contents::
    :local:

------------------------------------------------------------------------------
Code of Conduct
------------------------------------------------------------------------------

As a freedesktop.org project, libinput follows the `freedesktop.org
Contributor Covenant <https://www.freedesktop.org/wiki/CodeOfConduct>`_.

Please conduct yourself in a respectful and civilised manner when
interacting with community members on mailing lists, IRC, or bug trackers.
The community represents the project as a whole, and abusive or bullying
behaviour is not tolerated by the project.

------------------------------------------------------------------------------
Contact
------------------------------------------------------------------------------

Questions can be asked on ``#wayland-devel`` on freenode or on the
`wayland-devel@lists.freedesktop.org
<https://lists.freedesktop.org/mailman/listinfo/wayland-devel>`_ mailing
list.

For IRC, ping user ``whot`` (Peter Hutterer, the libinput maintainer) though
note that he lives on UTC+10 and thus the rest of the world is out of sync
by default ;)

For anything that appears to be device specific and/or related to a new
feature, just file `an issue in our issue tracker
<https://gitlab.freedesktop.org/libinput/libinput/issues>`_. It's usually the
most efficient way to get answers.

------------------------------------------------------------------------------
What to work on?
------------------------------------------------------------------------------

If you don't already know what you want to improve or fix with libinput,
then a good way of finding something is to search for the ``help needed``
tag in our `issue tracker <https://gitlab.freedesktop.org/libinput/libinput/issues?label_name%5B%5D=help+needed>`_.
These are issues that have been triaged to some degree and deemed to be a
possible future feature to libinput. 

.. note:: Some of these issue may require specific hardware to reproduce.

Another good place to help out with is the documentation. For anything you
find in these pages that isn't clear enough please feel free to reword it
and add what is missing.

------------------------------------------------------------------------------
Getting the code
------------------------------------------------------------------------------

The :ref:`building_libinput` have all the details but the short solution
will be:

::

     $> git clone https://gitlab.freedesktop.org/libinput/libinput
     $> cd libinput
     $> meson --prefix=/usr builddir/
     $> ninja -C builddir/
     $> sudo ninja -C builddir/ install

You can omit the last step if you only want to test locally.

------------------------------------------------------------------------------
Working on the code
------------------------------------------------------------------------------

libinput has a roughly three-parts architecture: 

-  the front-end code which handles the ``libinput_some_function()`` API calls in ``libinput.c``
-  the generic evdev interface handling which maps those API calls to the
   backend calls (``evdev.c``). 
- there are device-specific backends which do most of the actual work -
  ``evdev-mt-touchpad.c`` is the one for touchpads for example.

In general, things that only affect the internal workings of a device only
get implemented in the device-specific backend. You only need to touch the
API when you are adding configuration options. For more details, please read
the :ref:`architecture` document. There's also a `blog post describing the
building blocks
<https://who-t.blogspot.com/2019/03/libinputs-internal-building-blocks.html>`_
that may help to understand how it all fits together.

Documentation is in ``/doc/api`` for the doxygen-generated API documentation.
These are extracted from the libinput source code directly. The
documentation you're reading right now is in ``/doc/user`` and generated with
sphinx. Simply running ``ninja -C builddir`` will rebuild it and the final
product ends up in ``builddir/Documentation``.

------------------------------------------------------------------------------
Testing the code
------------------------------------------------------------------------------

libinput provides a bunch of :ref:`tools` to debug any changes - without
having to install libinput.

The two most useful ones are :ref:`libinput debug-events
<libinput-debug-events>` and :ref:`libinput debug-gui <libinput-debug-gui>`.
Both tools can be run from the build directory directly and are great for
quick test iterations::

  $> sudo ./builddir/libinput-debug-events --verbose
  $> sudo ./builddir/libinput-debug-gui --verbose

The former provides purely textual output and is useful for verifying event
streams from buttons, etc. The latter is particularly useful when you are
trying to debug pointer movement or placement. ``libinput debug-gui`` will
also visualize the raw data from the device so you can compare pointer
behavior with what comes from the kernel.

These tools create a new libinput context and will not affect your session's
behavior. Only once you've installed libinput and restarted your session
will your changes affect the X server/Wayland compositor.

Once everything seems to be correct, it's time to run the
:ref:`test-suite`::

  $> sudo ./builddir/libinput-test-suite

This test suite can take test names etc. as arguments, have a look at
:ref:`test-suite` for more info. There are a bunch of other tests that are
run by the CI on merge requests, you can run those locally with ::

  $> sudo ninja -C builddir check

So it always pays to run that before submitting. This will also run the code
through valgrind and pick up any memory leaks.

------------------------------------------------------------------------------
Submitting Code
------------------------------------------------------------------------------

Any patches should be sent via a Merge Request (see the `GitLab docs
<https://docs.gitlab.com/ce/gitlab-basics/add-merge-request.htm>`_)
in the `libinput GitLab instance hosted by freedesktop.org
<https://gitlab.freedesktop.org/libinput/libinput>`_.

To submit a merge request, you need to

- `Register an account <https://gitlab.freedesktop.org/users/sign_in>`_ in
  the freedesktop.org GitLab instance.
- `Fork libinput <https://gitlab.freedesktop.org/libinput/libinput/forks/new>`_
  into your username's namespace
- Get libinput's main repository: ::

    git clone https://gitlab.freedesktop.org/libinput/libinput.git

- Add the forked git repository to your remotes (replace ``USERNAME``
  with your username): ::

    cd /path/to/libinput.git
    git remote add gitlab git@gitlab.freedesktop.org:USERNAME/libinput.git
    git fetch gitlab

- Push your changes to your fork: ::

    git push gitlab BRANCHNAME

- Submit a merge request. The URL for a merge request is: ::

    https://gitlab.freedesktop.org/USERNAME/libinput/merge_requests

  Select your branch name to merge and ``libinput/libinput`` ``master`` as target branch.

------------------------------------------------------------------------------
Commit History
------------------------------------------------------------------------------

libinput strives to have a
`linear, 'recipe' style history <http://www.bitsnbites.eu/git-history-work-log-vs-recipe/>`_
This means that every commit should be small, digestible, stand-alone, and
functional. Rather than a purely chronological commit history like this: ::

	doc: final docs for view transforms
	fix tests when disabled, redo broken doc formatting
	better transformed-view iteration (thanks Hannah!)
	try to catch more cases in tests
	tests: add new spline test
	fix compilation on splines
	doc: notes on reticulating splines
	compositor: add spline reticulation for view transforms

We aim to have a clean history which only reflects the final state, broken up
into functional groupings: ::

	compositor: add spline reticulation for view transforms
	compositor: new iterator for view transforms
	tests: add view-transform correctness tests
	doc: fix Doxygen formatting for view transforms

This ensures that the final patch series only contains the final state,
without the changes and missteps taken along the development process.

The first line of a commit message should contain a prefix indicating
what part is affected by the patch followed by one sentence that
describes the change. For example: ::

	touchpad: add software button behavior
	fallback: disable button debouncing on device foo

If in doubt what prefix to use, look at other commits that change the
same file(s) as the patch being sent.

------------------------------------------------------------------------------
Commit Messages
------------------------------------------------------------------------------

Commit messages **must** contain a **Signed-off-by** line with your name
and email address. An example is: ::

    A description of this commit, and it's great work.

    Signed-off-by: Claire Someone <name@domain>

If you're not the patch's original author, you should
also gather S-o-b's by them (and/or whomever gave the patch to you.) The
significance of this is that it certifies that you created the patch, that
it was created under an appropriate open source license, or provided to you
under those terms. This lets us indicate a chain of responsibility for the
copyright status of the code. An example is: ::

    A description of this commit, and it's great work.

    Signed-off-by: Claire Someone <name@domain>
    Signed-off-by: Ferris Crab <name@domain>

When you re-send patches, revised or not, it would be very good to document the
changes compared to the previous revision in the commit message and/or the
merge request. If you have already received Reviewed-by or Acked-by tags, you
should evaluate whether they still apply and include them in the respective
commit messages. Otherwise the tags may be lost, reviewers miss the credit they
deserve, and the patches may cause redundant review effort.

For further reading, please see
`'on commit messages' <http://who-t.blogspot.de/2009/12/on-commit-messages.html>`_
as a general guideline on what commit messages should contain.

------------------------------------------------------------------------------
Coding Style
------------------------------------------------------------------------------

Please see the `CODING_STYLE.md
<https://gitlab.freedesktop.org/libinput/libinput/blob/master/CODING_STYLE.md>`_
document in the source tree.

------------------------------------------------------------------------------
Tracking patches and follow-ups
------------------------------------------------------------------------------

Once submitted to GitLab, your patches will be reviewed by the libinput
development team on GitLab. Review may be entirely positive and result in your
code landing instantly, in which case, great! You're done. However, we may ask
you to make some revisions: fixing some bugs we've noticed, working to a
slightly different design, or adding documentation and tests.

If you do get asked to revise the patches, please bear in mind the notes above.
You should use ``git rebase -i`` to make revisions, so that your patches
follow the clear linear split documented above. Following that split makes
it easier for reviewers to understand your work, and to verify that the code
you're submitting is correct.

A common request is to split single large patch into multiple patches. This can
happen, for example, if when adding a new feature you notice a bug in
libinput's core which you need to fix to progress. Separating these changes
into separate commits will allow us to verify and land the bugfix quickly,
pushing part of your work for the good of everyone, whilst revision and
discussion continues on the larger feature part. It also allows us to direct
you towards reviewers who best understand the different areas you are
working on.

When you have made any requested changes, please rebase the commits, verify
that they still individually look good, then force-push your new branch to
GitLab. This will update the merge request and notify everyone subscribed to
your merge request, so they can review it again.

There are also many GitLab CLI clients, if you prefer to avoid the web
interface. It may be difficult to follow review comments without using the
web interface though, so we do recommend using this to go through the review
process, even if you use other clients to track the list of available
patches.

