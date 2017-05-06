//========================================================================
//
//	Inferred Rendering Sample
//
//		by MJP  (mpettineo@gmail.com)
//      http://mynameismjp.wordpress.com
//		01/09/2010      
//
//========================================================================

using System;

namespace InferredRendering
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        static void Main(string[] args)
        {
            using (InferredRendering game = new InferredRendering())
            {
                game.Run();
            }
        }
    }
}

