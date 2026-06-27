# SNHU_CS-330_Project
Custom designed 3D scene, built using OpenGL API and prebuilt libraries.

**1. How do I approach designing software?**
   - What new design skills has your work on the project helped you to craft?
   - What design process did you follow for your project work?
   - How could tactics from your design approach be applied in future work?

   My approach to designing software is to focus on how the user will experience the finished product. Core functionality is important, but an inviting and easy to use interface is just as important. I wanted to build a product that was easy and fun for the user to explore. My design focused on being interactive and accessible, and provinding a seamless experience.

   One feature that incorporated that I am proud of is that I adpated the user interface to support left-handed users. Such that the keyboard supports either WASD control, or Arrow-Key + PgUp/PgDn controls. Thus a user would be able to use a laptop trackpad with their left-hand while navigating with right.

   Technical debts I still have with this project is that I need to add a graphic overlay with control descriptions, support for color-blindness, further accessibility controls, and a development mode that could allow users to extract data about the scene and manipulate certain environment variables from within the render window.

   The analysis of the UI/UX features of this application would definitely be useful to me in my future as a developer, as this is the single most important aspect of production level software development. After-all, if a user can't use the program, it's not worth much. Even API's that run on the backend aren't useful if developers can't interact with them easily if not at all.

**2. How do I approach developing programs?**
   - What new development strategies did you use while working on your 3D scene?
   - How did iteration factor into your development?
   - How has your approach to developing code evolved throughout the milestones, which led you to the project’s completion?
  
   One of my first approaches to designing software is looking for ways to encapsulate functionality, building tools to help the developer through repetitive tasks. This is an important cornerstone for me that I've derived from Object Oriented Principles as it has significantly sped up my development when building programs. I spend significantly less time repeating keystrokes, or debugging blocks of code that were copied but not fully patched.

   I focussed primarily on building helper functions throughout this program to assist development. Where I have functions to convert color values, build shapes, and process specific user inputs. Further, I exercised my implementation of control loops and iterations. Using *for* loops to build objects for me without having to individually write code, and in this case I avoided most control loops to make sure I didn't trap the render or the user within a infinite or broken loop. I opted to build this program to fail fast and loud, rather than to try and incorporate fancy logic.

**3. How can computer science help me in reaching my goals?**
   - How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future educational pathway?
   - How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future professional pathway?

   Though I'm in my senior year, I still can't be certain of where this path will take me, professionally or educationally, but the one constant of my journey thus far, is that I'm always learning something new that's valuable. What I imagine I can take away from this is a better understanding of how computer graphics work if I ever become a graphic designer or digital artist. I've learned how to develop in Visual Studio, and I've strengthened my understanding of OOP principles, as well as my C++ skills. I'm also well on my way to being able to build my own video game, if I ever wanted to build one from scratch, without using a development platform.
