# Cassowary, lite

A C interface to Cassowary - with some reduction, cleaning,
orthogonalization.

All of the credit to the whiz-bang's due to the original authors of Cassowary:
respectively, Greg J. Badros; Alan Borning; Peter J. Stuckey. The
sharp edges and hole-riddled hacks should, on the other hand, be laid
directly at my doormat.

This library's a fairly simple wrapper layer that exposes the
user-facing object hierarchy within Cassowary in a C-compatible (and
C-interfacing FFI-compatible) interface. (There *is* in fact one
significant difference that'll arouse a great deal of suspicion if
you're a Cassowary veteran: ClLinearEquations and ClLinearInequalities
have been combined into the same class with the help of the abstract
relation operator facility ClConstraint exposes - and
ClLinearInequalities assiduously uses as well; even to the point of
obviating the need for ClLinearEquations.)

I hope I've done a fair job of providng something predictable and
pleasing to the touch.

ClC.h ostensibly does the same job, but I'd argue  that
the interface it exposes (ClConstraints must be instantiated from
strings to be parsed, and very little of the composing abstractions
underneath are shown) requires we round-trip through an expensive
intermediate representation if we wish to go straight from meaningful
data already represented in structures, to Expressions in Cassowary's
own.

## Building *and using* Cassowary, lite:

1. Copy libcassowary's static libraries *and* the "c++" subdirectory from Cassowary's source
   distribution into the root of the source tree; you may need help
   getting Cassowary to compile, and so I've documented my experiences below.

2. **make**; libcassowary-lite should pop up in both shared and
   dynamic forms.

That's it - you'll have more fun wrangling with the interface I've
cooked up; cassowary-lite.h *should* be spare enough to be easily
understood. Bear in mind the standard narrative of a Cassowary
application: Variables are either directly involved in
StayOrEditConstraints, which assert invariants (as long as Cassowary
can assert the invariants, at the very least); or shoved into Expressions, and in turn into
Relations, which are then put into SimplexSolvers to have their
constraints met - at which point the values you need'll be back in the
Variables.

If you so wish to code UI event-handling patterns around this library,
there exists a mutable-state annex that allows you to edit the
variables within StayOrEditConstraints, and re-solve with alterations
made as required - the original "SuggestValues()" indicates that
alterations may occur, but I'd argue  more important is to also indicate that
they occur when edits are made!


## Getting Cassowary to compile, if you haven't yet cut your teeth on it

1. **Extract the C++ source directory from the rest of the Cassowary
   source distribution.** You'll find it under the 'c++' folder - we're
   in luck, since this subdirectory's self-contained: we can build
   precisely what we need with only it. Copy that into the root of
   *this* source tree.

2. **Add -fPIC to CPPFLAGS** in Makefile.linux - the Makefile target
   we're using builds *both* static and shared libraries, while the
   proper directives are only issued for the former, and

    *. **Surgically remove ClReader from the compile process** - the
        directives issued to flex and yacc no longer do what they should,
        and you'll end up with issues with both that you shouldn't quite
        frankly have to care about. Do both steps in one fell swoop - patch
        Makefile.linux 

4. **ln -s . cassowary** - Cl.h *requires* that headers be in a
   non-existent subdirectory called 'cassowary'; since they're in the
   current directory, though, we'll just fake up the subdirectory's
   existence.

5. **make -f Makefile.linux all-notests**: 'notests' because the
   functionality that ClReader provides is being tested in those as
   well - which we've just stripped out. At this point you should have
   *libcassowary.[a | so]* and *libcccassowary.[a | so]* built in the
   directory - at which point you should be able to copy the libraries
   back up to the top, and proceed with the rest of the instructions.

## Licensing.

Public domain. I don't trust myself enough to believe that even *a*
ceremony establishing my rights is required.
