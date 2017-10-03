Questions
===============

Questions (and potential answers) appear during attempting to implement the
Executor Context proposal.


1. How does the executor and the context  interact with each other?
 
    e.g: If the topology and placement information is on the EC, 
         how does the executor places the thread?

    Currently: An eR.place_thread method is added to the EC and called 
      from the E.


Glossary/Abbreviations
-----------------------

EC : Executor context
E  : Executor
ER : Execution resource
eR : Instance of an execution resource


