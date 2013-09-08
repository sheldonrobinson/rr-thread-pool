/*
Copyright (c) 2013, Riccardo Ressi
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of Riccardo Ressi nor the names of its contributors may be
used to endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DOCUMENTATION_H
#define DOCUMENTATION_H

/**
 * @mainpage RR Thread Pool
 *
 * This is a reference implementation of the Thread Pool design pattern.
 *
 * The implementation satisfy the following requirements:
 * - Modern C++ (shared pointers, RAII teqniques, clean code).
 * - Easily portable over different platforms.
 * - Baseline, didactical and clean implementation  with no exoteric extras.
 *
 * Main classes:
 * - @ref IThreadPool
 * - @ref IMessageQueue
 * - @ref ITask
 */

/**
 * @defgroup threading-base Base
 *
 * This groop contains base (and low level) classes to implement multi-threaded
 * application.
 *
 * On most common use-cases is better not to use this classes but choose to
 * use more elegant and evoluted classes like @ref MessageQueue and @ref
 * ThreadQueue.
 */

/**
 * @defgroup raii RAII
 *
 * This groop contains a set of classes designed to be used in order to write
 * code that follows the design guinelines described by the popular design
 * pattern @ref RAII.
 */

/**
  * @page RAII Resource Acquisition Is Initialization
  *
  * Resource Acquisition Is Initialization (RAII) is a programming idiom used
  * in several object-oriented languages like C++, D, Ada and Vala. The
  * technique was invented by Bjarne Stroustrup to deal with resource
  * allocation and deallocation in C++.
  *
  * In this language, if an exception is thrown, and proper exception handling
  * is in place, the only code that will be executed for the current scope are
  * the destructors of objects declared in that scope. Resource management
  * therefore needs to be tied to the lifespan of suitable objects in order to
  * gain automatic allocation and reclamation.
  *
  * Resources are acquired during initialization, when there is no chance of
  * them being used before they are available, and released with the
  * destruction of the same objects, which is guaranteed to take place even in
  * case of errors.
  *
  * Taken from: http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization
  */

#endif // DOCUMENTATION_H
