//========================================================================
//
//	PIXWithXNA Tutorial
//
//		by MJP  (mpettineo@gmail.com)
//		08/21/09      
//
//========================================================================

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Diagnostics;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;

namespace SampleCommon
{
    /// <summary>
    /// Contains static methods for creating events in markers
    /// used during PIX profiling.
    /// </summary>
    public static class PIXHelper
    {
        #region private fields

        // Keeps track of our BeginEvent/EndEvent pairs
        private static int eventCount = 0;

        #endregion

        #region Public static methods

        /// <summary>
        /// Determines whether PIX is attached to the running process
        /// </summary>
        /// <returns>true if PIX is attached, false otherwise</returns>
        public static bool IsPIXAttached()
        {
            return (D3DPERF_GetStatus() > 0);
        }

        /// <summary>
        /// Calling this method will prevent PIX from being able
        /// to attach to this process.
        /// </summary>
        public static void DisablePIXProfiling()
        {
            D3DPERF_SetOptions(1);
        }

        /// <summary>
        /// Marks the beginning of a PIX event.  Events are used to group
        /// together a series of related API calls, for example a series of
        /// commands needed to draw a single model.  Events can be nested.
        /// </summary>
        /// <param name="eventName">The name of the event</param>
        public static void BeginEvent(string eventName)
        {
            D3DPERF_BeginEvent(Color.Black.PackedValue, eventName);
            eventCount++;
        }

        /// <summary>
        /// Marks the end of a PIX event.
        /// </summary>
        public static void EndEvent()
        {
            // Make sure we haven't called EndEvent more times
            // than we've called BeginEvent
            Debug.Assert(eventCount >= 1);

            D3DPERF_EndEvent();
            eventCount--;
        }

        /// <summary>
        /// Adds a marker in PIX.  A marker is used to indicate that single
        /// instantaneous event has occured.
        /// </summary>
        /// <param name="markerName"></param>
        public static void SetMarker(string markerName)
        {
            D3DPERF_SetMarker(Color.Black.PackedValue, markerName);
        }

        #endregion

        #region Extension methods

        #region SpriteBatch extension methods

        /// <summary>
        /// Begins SpriteBatch drawing, and marks the beginning of a PIX event.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use</param>
        /// <param name="blendMode">Blending options to use when rendering</param>
        /// <param name="userMessage">A string to append to the PIX event name</param>
        public static void BeginPIX(this SpriteBatch spriteBatch,
                                    string userMessage)
        {
            BeginEvent(MakeName("SpriteBatch Drawing", userMessage));
            SetMarker("SpriteBatch.Begin Called");
            spriteBatch.Begin();
        }

        /// <summary>
        /// Begins SpriteBatch drawing, and marks the beginning of a PIX event.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use</param>
        /// <param name="blendMode">Blending options to use when rendering</param>
        /// <param name="userMessage">A string to append to the PIX event name</param>
        public static void BeginPIX(this SpriteBatch spriteBatch,
                                    SpriteBlendMode blendMode,
                                    string userMessage)
        {
            BeginEvent(MakeName("SpriteBatch Drawing", userMessage));
            SetMarker("SpriteBatch.Begin Called");
            spriteBatch.Begin(blendMode);
        }

        /// <summary>
        /// Begins SpriteBatch drawing, and marks the beginning of a PIX event.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use</param>
        /// <param name="blendMode">Blending options to use when rendering</param>
        /// <param name="sortMode">Sorting options to use when rendering</param>
        /// <param name="stateMode">Rendering state options</param>
        /// <param name="userMessage">A string to append to the PIX event name</param>
        public static void BeginPIX(this SpriteBatch spriteBatch,
                                    SpriteBlendMode blendMode,
                                    SpriteSortMode sortMode,
                                    SaveStateMode stateMode,
                                    string userMessage)
        {
            BeginEvent(MakeName("SpriteBatch Drawing", userMessage));
            SetMarker("SpriteBatch.Begin Called");
            spriteBatch.Begin(blendMode, sortMode, stateMode);
        }

        /// <summary>
        /// Begins SpriteBatch drawing, and marks the beginning of a PIX event.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use</param>
        /// <param name="blendMode">Blending options to use when rendering</param>
        /// <param name="sortMode">Sorting options to use when rendering</param>
        /// <param name="stateMode">Rendering state options</param>
        /// <param name="transformMatrix">A matrix to apply to position, rotation, scale, and depth data passed to SpriteBatch.Draw</param>
        /// <param name="userMessage">A string to append to the PIX event name. Pass null for no message.</param>
        public static void BeginPIX(this SpriteBatch spriteBatch, 
                                    SpriteBlendMode blendMode, 
                                    SpriteSortMode sortMode,
                                    SaveStateMode stateMode,
                                    Matrix transformMatrix,
                                    string userMessage)
        {
            BeginEvent(MakeName("SpriteBatch Drawing", userMessage));
            SetMarker("SpriteBatch.Begin Called");
            spriteBatch.Begin(blendMode, sortMode, stateMode, transformMatrix);
        }

        /// <summary>
        /// Draws a texture, and sets a marker in PIX.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use.</param>
        /// <param name="texture">The sprite texture.</param>
        /// <param name="destinationRectangle">A rectangle specifying where the sprite should be drawn on the screen.</param>
        /// <param name="color">The color channel modulation to use. Pass Color.White for no tinting.</param>
        /// <param name="userMessage">A string to append to the PIX marker name. Pass null for no message.</param>
        public static void DrawPIX(this SpriteBatch spriteBatch,
                                    Texture2D texture,
                                    Rectangle destinationRectangle,
                                    Color color,                                   
                                    string userMessage)
        {
            SetMarker(MakeName("SpriteBatch.Draw Called", userMessage));
            spriteBatch.Draw(texture, destinationRectangle, color);
        }

        /// <summary>
        /// Draws a texture, and sets a marker in PIX.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use.</param>
        /// <param name="texture">The sprite texture.</param>
        /// <param name="position">The screen-space location to draw the sprite at.</param>
        /// <param name="color">The color channel modulation to use. Pass Color.White for no tinting.</param>
        /// <param name="userMessage">A string to append to the PIX marker name. Pass null for no message.</param>
        public static void DrawPIX(this SpriteBatch spriteBatch,
                                    Texture2D texture,
                                    Vector2 position,
                                    Color color,
                                    string userMessage)
        {
            SetMarker(MakeName("SpriteBatch.Draw Called", userMessage));
            spriteBatch.Draw(texture, position, color);
        }

        /// <summary>
        /// Draws a texture, and sets a marker in PIX.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use.</param>
        /// <param name="texture">The sprite texture.</param>
        /// <param name="destinationRectangle">A rectangle specifying where the sprite should be drawn on the screen.</param>
        /// <param name="sourceRectangle">A rectangle specifying which portion of the texture to draw. Pass null to draw the whole texture.</param>
        /// <param name="color">The color channel modulation to use. Pass Color.White for no tinting.</param>
        /// <param name="userMessage">A string to append to the PIX marker name. Pass null for no message.</param>
        public static void DrawPIX(this SpriteBatch spriteBatch,
                                    Texture2D texture,
                                    Rectangle destinationRectangle,
                                    Rectangle? sourceRectangle,
                                    Color color,                                    
                                    string userMessage)
        {
            SetMarker(MakeName("SpriteBatch.Draw Called", userMessage));
            spriteBatch.Draw(texture, destinationRectangle, sourceRectangle, color);
        }

        /// <summary>
        /// Draws a texture, and sets a marker in PIX.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use.</param>
        /// <param name="texture">The sprite texture.</param>
        /// <param name="position">The screen-space location to draw the sprite at.</param>
        /// <param name="sourceRectangle">A rectangle specifying which portion of the texture to draw. Pass null to draw the whole texture.</param>
        /// <param name="color">The color channel modulation to use. Pass Color.White for no tinting.</param>
        /// <param name="userMessage">A string to append to the PIX marker name. Pass null for no message.</param>
        public static void DrawPIX(this SpriteBatch spriteBatch,
                                    Texture2D texture,
                                    Vector2 position,
                                    Rectangle? sourceRectangle,
                                    Color color,                                   
                                    string userMessage)
        {
            SetMarker(MakeName("SpriteBatch.Draw Called", userMessage));
            spriteBatch.Draw(texture, position, sourceRectangle, color);
        }

        /// <summary>
        /// Draws a texture, and sets a marker in PIX.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use.</param>
        /// <param name="texture">The sprite texture.</param>
        /// <param name="destinationRectangle">A rectangle specifying where the sprite should be drawn on the screen.</param>
        /// <param name="sourceRectangle">A rectangle specifying which portion of the texture to draw. Pass null to draw the whole texture.</param>
        /// <param name="color">The color channel modulation to use. Pass Color.White for no tinting.</param>
        /// <param name="rotation">The angle, in radians, to roate the sprite about the origin.</param>
        /// <param name="origin">The origin for rotation and scaling.</param>        
        /// <param name="effects">Rotations to apply prior to rendering.</param>
        /// <param name="layerDepth">Specifies the sorting depth of the sprite.  Must be between 0 and 1, inclusive.</param>
        /// <param name="userMessage">A string to append to the PIX marker name. Pass null for no message.</param>
        public static void DrawPIX(this SpriteBatch spriteBatch,
                                    Texture2D texture,                                    
                                    Rectangle destinationRectangle,
                                    Rectangle? sourceRectangle,
                                    Color color,
                                    float rotation,
                                    Vector2 origin,
                                    SpriteEffects effects,
                                    float layerDepth,
                                    string userMessage)
        {
            SetMarker(MakeName("SpriteBatch.Draw Called", userMessage));
            spriteBatch.Draw(texture, destinationRectangle, sourceRectangle, color, rotation, origin, effects, layerDepth);
        }

        /// <summary>
        /// Draws a texture, and sets a marker in PIX.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use.</param>
        /// <param name="texture">The sprite texture.</param>
        /// <param name="position">The screen-space location to draw the sprite at.</param>
        /// <param name="sourceRectangle">A rectangle specifying which portion of the texture to draw. Pass null to draw the whole texture.</param>
        /// <param name="color">The color channel modulation to use. Pass Color.White for no tinting.</param>
        /// <param name="rotation">The angle, in radians, to roate the sprite about the origin.</param>
        /// <param name="origin">The origin for rotation and scaling.</param>
        /// <param name="scale">Scales the width and height of the sprite.</param>
        /// <param name="effects">Rotations to apply prior to rendering.</param>
        /// <param name="layerDepth">Specifies the sorting depth of the sprite.  Must be between 0 and 1, inclusive.</param>
        /// <param name="userMessage">A string to append to the PIX marker name. Pass null for no message.</param>
        public static void DrawPIX(this SpriteBatch spriteBatch,
                                    Texture2D texture,
                                    Vector2 position,
                                    Rectangle? sourceRectangle,
                                    Color color,
                                    float rotation,
                                    Vector2 origin,
                                    float scale,
                                    SpriteEffects effects,
                                    float layerDepth,
                                    string userMessage)
        {
            SetMarker(MakeName("SpriteBatch.Draw Called", userMessage));
            spriteBatch.Draw(texture, position, sourceRectangle, color, rotation, origin, scale, effects, layerDepth);
        }  

        /// <summary>
        /// Draws a texture, and sets a marker in PIX.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use.</param>
        /// <param name="texture">The sprite texture.</param>
        /// <param name="position">The screen-space location to draw the sprite at.</param>
        /// <param name="sourceRectangle">A rectangle specifying which portion of the texture to draw. Pass null to draw the whole texture.</param>
        /// <param name="color">The color channel modulation to use. Pass Color.White for no tinting.</param>
        /// <param name="rotation">The angle, in radians, to roate the sprite about the origin.</param>
        /// <param name="origin">The origin for rotation and scaling.</param>
        /// <param name="scale">Scales the width and height of the sprite.</param>
        /// <param name="effects">Rotations to apply prior to rendering.</param>
        /// <param name="layerDepth">Specifies the sorting depth of the sprite.  Must be between 0 and 1, inclusive.</param>
        /// <param name="userMessage">A string to append to the PIX marker name. Pass null for no message.</param>
        public static void DrawPIX(this SpriteBatch spriteBatch,
                                    Texture2D texture,
                                    Vector2 position,
                                    Rectangle? sourceRectangle,
                                    Color color,
                                    float rotation,
                                    Vector2 origin,
                                    Vector2 scale,
                                    SpriteEffects effects,
                                    float layerDepth,
                                    string userMessage)
                                    
        {
            SetMarker(MakeName("SpriteBatch.Draw Called", userMessage));
            spriteBatch.Draw(texture, position, sourceRectangle, color, rotation, origin, scale, effects, layerDepth);            
        }        

        /// <summary>
        /// Ends SpriteBatch drawing, and ends the PIX event.
        /// </summary>
        /// <param name="spriteBatch">The SpriteBatch to use.</param>
        public static void EndPix(this SpriteBatch spriteBatch)
        {
            SetMarker("SpriteBatch.End Called");
            spriteBatch.End();
            EndEvent();
        }

        #endregion

        #region ModelMesh extension methods

        /// <summary>
        /// Draws all of the ModelMeshPart objects in the mesh, and creates an event in PIX.
        /// </summary>
        /// <param name="mesh">The mesh to draw</param>
        /// <param name="userMessage">A string to append to the PIX event name. Pass null for no message.</param>
        public static void DrawPIX(this ModelMesh mesh, string userMessage)
        {
            BeginEvent(MakeName("ModelMesh.Draw Called", userMessage));
            mesh.Draw();
            EndEvent();
        }

        /// <summary>
        /// Draws all of the ModelMeshPart objects in the mesh, and creates an event in PIX.
        /// </summary>
        /// <param name="mesh">The mesh to draw</param>
        /// <param name="stateMode">The save state options to pass to the Effect for each part</param>
        /// <param name="userMessage">A string to append to the PIX event name. Pass null for no message.</param>
        public static void DrawPIX(this ModelMesh mesh, SaveStateMode stateMode, string userMessage)
        {
            BeginEvent(MakeName("ModelMesh.Draw Called", userMessage));
            mesh.Draw(stateMode);
            EndEvent();
        }

        #endregion

        #endregion

        #region Private methods

        private static string MakeName(string baseName, string userMessage)
        {
            if (!string.IsNullOrEmpty(userMessage))
                return baseName + " - " + userMessage;
            else
                return baseName;
        }

        #endregion

        #region PInvokes

        [DllImport("d3d9.dll")]
        private static extern uint D3DPERF_GetStatus();

        [DllImport("d3d9.dll")]
        private static extern void D3DPERF_SetOptions(uint dwOptions);

        [DllImport("d3d9.dll", CharSet = CharSet.Unicode)]
        private static extern int D3DPERF_BeginEvent(uint col, string wszName);

        [DllImport("d3d9.dll")]
        private static extern int D3DPERF_EndEvent();

        [DllImport("d3d9.dll", CharSet = CharSet.Unicode)]
        private static extern void D3DPERF_SetMarker(uint col, string wszName);

        #endregion
    }
}