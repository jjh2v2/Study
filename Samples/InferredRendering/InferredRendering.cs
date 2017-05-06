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
using System.Collections.Generic;
using System.Diagnostics;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.GamerServices;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using Microsoft.Xna.Framework.Media;
using Microsoft.Xna.Framework.Net;
using Microsoft.Xna.Framework.Storage;

using SampleCommon;

namespace InferredRendering
{
    /// <summary>
    /// The main class for the sample.  Loads all content, initalizes the scene, and
    /// handles all rendering and updating.
    /// </summary>
    public class InferredRendering : Game
    {
        // Constants
        const int ScreenWidth = 1280;
        const int ScreenHeight = 720;        
        static readonly Vector3 AmbientLight = new Vector3(0.0f, 0.0f, 0.0f);

        // These values are multiplied with ScreenWidth and ScreenHeight to
        // determine the size of the G-Buffer and L-Buffer
        static readonly Vector2[] GBufferSizes = { new Vector2(1.0f, 1.0f), new Vector2(0.5f, 1.0f), new Vector2(0.5f, 0.5f) };

        // The OutputMasks array is used to create a stipple pattern for the transparents 
        // during the G-Buffer pass.  For each pixel the shader determines its location within
        // a 2x2 quad, then takes the appropriate value from the mask and feeds it to the clip()
        // intrinsic.  So -1 results in no pixel being written.  The filter offsets array is
        // used by transparents during the composite pass to properly sample the nearest 4
        // pixels according to the output mask used.
        const int NumMasks = 3;
        static readonly Vector4[] OutputMasks = { new Vector4(1, -1, -1, -1), new Vector4(-1, 1, -1, -1), new Vector4(-1, -1, 1, -1) };
        static readonly Vector4[] XFilterOffsets = { new Vector4(0, -1, 0, -1), new Vector4(-1, 0, -1, 0), new Vector4(0, -1, 0, -1) };
        static readonly Vector4[] YFilterOffsets = { new Vector4(0, 0, -1, -1), new Vector4(0, 0, -1, -1), new Vector4(-1, -1, 0, 0) };
      
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;

        // These two make up our G-Buffer.  First target contains 16 bits for depth,
        // and 16 bits for holding an instance ID.
        RenderTarget2D depthIDBuffer;
        RenderTarget2D normalSpecularBuffer;

        // The L-Buffer contains the results of our lighting pass.  Shadow Map
        // contains light-space depth for the sunlight.
        RenderTarget2D lightBuffer;
        RenderTarget2D shadowMap;
        
        // The final result of the composite pass
        RenderTarget2D colorBuffer;

        // We use up to three DS buffers, since MSAA type and size need to match
        // the current RenderTarget.
        DepthStencilBuffer msaaDS;
        DepthStencilBuffer nonMSAADS;
        DepthStencilBuffer shadowMapDS;
        
        // Used to determine the G-Buffer and L-Buffer dimensions        
        GBufferSize gBufferSize = GBufferSize.FullSize;
        Vector2 gBufferDimensions = new Vector2(ScreenWidth, ScreenHeight);
        
        // The actual Model assets loaded from file
        Model shipModel;
        Model lightSphere;
        Model floor;

        // Texture assets loaded from file
        Texture2D saucerNormal;
        Texture2D floorNormal;

        // Effects loaded from file
        Effect gBufferEffect;
        Effect lightEffect;
        Effect compositeEffect;
        Effect transparentEffect;
        Effect shadowMapEffect;
        Effect spriteEffect;

        // SpriteFont using Arial font set
        SpriteFont font;
        
        // Our list of instances that we'll be drawing
        List<ModelInstance> opaques = new List<ModelInstance>();
        List<ModelInstance> transparents = new List<ModelInstance>();
        List<ModelInstance> sortedTransparents = new List<ModelInstance>();

        // Our list of lights in the scene
        List<Light> lights = new List<Light>();
        DirectionalLight sunlight;
        OrthographicCamera lightCam;        
        
        // The camera we'll use for drawing the scene
        FirstPersonCamera camera = new FirstPersonCamera(MathHelper.PiOver4, 16.0f / 9.0f, 1.0f, 5000.0f);

        // We'll use this for drawing the directional lights
        FullScreenQuad quad;

        // Input state from the previous frame
        KeyboardState lastKBState;
        MouseState lastMouseState;

        // This stuff is for calculating the FPS, with filtering
        const int DTFilterSize = 64;
        Stopwatch stopwatch = new Stopwatch();
        long lastTime = 0;
        long[] dtFilter = new long[DTFilterSize];
        int fps;
        int dtIndex = 0;

        // Runtime tweakables
        bool enableMSAA = false;
        bool enableNormalMaps = true;
        bool enablePointLights = true;       
        TransparencyMode transparencyMode = TransparencyMode.ForwardRendered;
        ShadowMapSize shadowMapSize = ShadowMapSize.Size2048;   

        /// <summary>
        /// Main constructor.  Sets up the GraphicsDeviceManager and Game class to our liking.
        /// </summary>
        public InferredRendering()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
            this.IsFixedTimeStep = false;

            graphics.MinimumVertexShaderProfile = ShaderProfile.VS_3_0;
            graphics.MinimumPixelShaderProfile = ShaderProfile.PS_3_0;
            graphics.PreferredDepthStencilFormat = DepthFormat.Depth24Stencil8;
            graphics.PreferredBackBufferWidth = ScreenWidth;
            graphics.PreferredBackBufferHeight = ScreenHeight;            
            graphics.ApplyChanges();

            stopwatch.Start();
        }

        /// <summary>
        /// Loads of all of our assets, and creates any graphical resources
        /// needed for runtime.
        /// </summary>
        protected override void LoadContent()
        {            
            spriteBatch = new SpriteBatch(GraphicsDevice);

            shipModel = Content.Load<Model>("Saucer");
            lightSphere = Content.Load<Model>("LightSphere");
            floor = Content.Load<Model>("Floor");            

            saucerNormal = Content.Load<Texture2D>("Rocky_Normal");
            floorNormal = Content.Load<Texture2D>("Floor_Normal");
                    
            gBufferEffect = Content.Load<Effect>("GBuffer");
            lightEffect = Content.Load<Effect>("Light");
            compositeEffect = Content.Load<Effect>("Composite");
            transparentEffect = Content.Load<Effect>("Transparent");
            shadowMapEffect = Content.Load<Effect>("ShadowMap");
            spriteEffect = Content.Load<Effect>("Sprite");

            font = Content.Load<SpriteFont>("Arial");

            quad = new FullScreenQuad(GraphicsDevice);

            CreateInstances();

            CreateLights();

            CreateRenderTargets();

            camera.Position = new Vector3(0, 400, 2000);
        }

        /// <summary>
        /// Creates ModelInstances and places them in the scene
        /// </summary>
        private void CreateInstances()
        {
            // We'll make 3 instances of the "Saucer" model and give them normal maps.
            // We'll copy the rest of the properties from the model.
            BasicEffect shipEffect = (BasicEffect)shipModel.Meshes[0].Effects[0];
            shipEffect.SpecularPower *= 4;
            ModelInstance shipInstance = new ModelInstance(shipModel,
                                                shipEffect.Texture,
                                                saucerNormal,
                                                shipEffect.SpecularPower,
                                                shipEffect.SpecularColor,
                                                Vector2.One);
            shipInstance.Position = new Vector3(0, 100, 0);
            opaques.Add(shipInstance);

            shipInstance = new ModelInstance(shipModel,
                                                shipEffect.Texture,
                                                saucerNormal,
                                                shipEffect.SpecularPower,
                                                shipEffect.SpecularColor,
                                                Vector2.One);
            shipInstance.Position = new Vector3(0, 400, 0);
            opaques.Add(shipInstance);

            shipInstance = new ModelInstance(shipModel,
                                                shipEffect.Texture,
                                                saucerNormal,
                                                shipEffect.SpecularPower,
                                                shipEffect.SpecularColor,
                                                Vector2.One);
            shipInstance.Position = new Vector3(0, 700, 0);
            opaques.Add(shipInstance);

            // This will be an opaque plane
            BasicEffect floorEffect = (BasicEffect)floor.Meshes[0].Effects[0];
            ModelInstance floorInstance = new ModelInstance(floor,
                                                                floorEffect.Texture,
                                                                floorNormal,
                                                                floorEffect.SpecularPower,
                                                                floorEffect.SpecularColor,
                                                                new Vector2(4.0f));
            floorInstance.Position = new Vector3(0, 0, 0);
            opaques.Add(floorInstance);

            // We'll make two transparent instances of the same plane
            floorInstance = new ModelInstance(floor,
                                                floorEffect.Texture,
                                                floorNormal,
                                                floorEffect.SpecularPower,
                                                floorEffect.SpecularColor,
                                                            new Vector2(4.0f));
            floorInstance.Position = new Vector3(0, 250, 0);
            transparents.Add(floorInstance);

            floorInstance = new ModelInstance(floor,
                                                floorEffect.Texture,
                                                floorNormal,
                                                floorEffect.SpecularPower,
                                                floorEffect.SpecularColor,
                                                new Vector2(4.0f));
            floorInstance.Position = new Vector3(0, 500, 0);
            transparents.Add(floorInstance);            
        }

        /// <summary>
        /// Creates lights for the scene
        /// </summary>
        private void CreateLights()
        {
            PointLight light = new PointLight();
            light.Position = new Vector3(-500, 800, 500);
            light.Color = new Vector3(0.9f, 0.2f, 0.2f) * 0.5f;
            light.Range = 1200.0f;
            lights.Add(light);

            light = new PointLight();
            light.Position = new Vector3(-500, 800, -500);
            light.Color = new Vector3(0.2f, 0.9f, 0.2f) * 0.5f;
            light.Range = 1200.0f;
            lights.Add(light);

            light = new PointLight();
            light.Position = new Vector3(500, 800, -500);
            light.Color = new Vector3(0.2f, 0.2f, 0.9f) * 0.5f;
            light.Range = 1200.0f;
            lights.Add(light);

            light = new PointLight();
            light.Position = new Vector3(500, 800, 500);
            light.Color = new Vector3(0.2f, 0.7f, 0.7f) * 0.5f;
            light.Range = 1200.0f;
            lights.Add(light);

            sunlight = new DirectionalLight();
            sunlight.Color = new Vector3(0.9f, 0.9f, 0.6f) * 0.2f;
            sunlight.Direction = Vector3.Normalize(new Vector3(0.5f, -1.0f, -1.0f));
            lights.Add(sunlight);

            // Setup the virtual camera for sunlight shadowmap projection
            Vector3 lightPos = new Vector3(0, 350, 0) - (sunlight.Direction * 2000);
            Matrix lightView = Matrix.CreateLookAt(lightPos, lightPos + sunlight.Direction, Vector3.Up);

            Vector3 maxWS = new Vector3(1000.0f, 1000.0f, 1000.0f);
            Vector3 minWS = new Vector3(-1000.0f, 0.0f, -1000.0f);
            BoundingBox bbWS = new BoundingBox(minWS, maxWS);
            Vector3[] points = bbWS.GetCorners();
            Vector3.Transform(points, ref lightView, points);
            BoundingBox bbVS = BoundingBox.CreateFromPoints(points);

            lightCam = new OrthographicCamera(bbVS.Min.X, bbVS.Max.X, bbVS.Min.Y, bbVS.Max.Y, -bbVS.Max.Z, -bbVS.Min.Z);
            lightCam.ViewMatrix = lightView;
        }

        /// <summary>
        /// Creates render targets and DS buffers needed for rendering
        /// </summary>
        private void CreateRenderTargets()
        {
            Vector2 gBufferMultiplier = GBufferSizes[(int)gBufferSize];
            int gBufferWidth = (int)(gBufferMultiplier.X * ScreenWidth);
            int gBufferHeight = (int)(gBufferMultiplier.Y * ScreenHeight);
            gBufferDimensions = new Vector2(gBufferWidth, gBufferHeight);

            if (depthIDBuffer != null)
                depthIDBuffer.Dispose();
            depthIDBuffer = new RenderTarget2D(GraphicsDevice, gBufferWidth, gBufferHeight, 1, 
                                                SurfaceFormat.HalfVector2, RenderTargetUsage.PreserveContents);

            // We'll prefer Rgba1010102 for normals + specular exponent, since we only need 3 components
            // and it's nice to have the extra precision for normals.  If you needed another component, 
            // you could use Color and only use 8-bits for the normals.  Or if you wanted to check to see if
            // the GPU supports MRT with independed bit-depths, you could go with HalfVector4 and get
            // even more precision.
            SurfaceFormat normalSpecularFormat = SurfaceFormat.Color;
            GraphicsAdapter adapter = GraphicsAdapter.DefaultAdapter;
            SurfaceFormat adapterFormat = adapter.CurrentDisplayMode.Format;
            if (adapter.CheckDeviceFormat(DeviceType.Hardware, adapterFormat, TextureUsage.None, QueryUsages.None, 
                                            ResourceType.RenderTarget, SurfaceFormat.Rgba1010102))
                normalSpecularFormat = SurfaceFormat.Rgba1010102;

            if (normalSpecularBuffer != null)
                normalSpecularBuffer.Dispose();
            normalSpecularBuffer = new RenderTarget2D(GraphicsDevice, gBufferWidth, gBufferHeight, 1, 
                                                normalSpecularFormat, RenderTargetUsage.PreserveContents);
            
            if (lightBuffer != null)
                lightBuffer.Dispose();
            lightBuffer = new RenderTarget2D(GraphicsDevice, gBufferWidth, gBufferHeight, 1, 
                                                SurfaceFormat.HalfVector4, RenderTargetUsage.PreserveContents);

            if (colorBuffer != null)
                colorBuffer.Dispose();
            
            if (enableMSAA)
                colorBuffer = new RenderTarget2D(GraphicsDevice, ScreenWidth, ScreenHeight, 1,
                                                SurfaceFormat.Color, MultiSampleType.FourSamples, 0, 
                                                RenderTargetUsage.PreserveContents);
            else
                colorBuffer = new RenderTarget2D(GraphicsDevice, ScreenWidth, ScreenHeight, 1, 
                                                SurfaceFormat.Color, RenderTargetUsage.PreserveContents);

            nonMSAADS = GraphicsDevice.DepthStencilBuffer;

            if (msaaDS != null && msaaDS != nonMSAADS)
                msaaDS.Dispose();
            if (enableMSAA)
                msaaDS = new DepthStencilBuffer(GraphicsDevice, ScreenWidth, ScreenHeight, 
                                                    DepthFormat.Depth24Stencil8,
                                                    MultiSampleType.FourSamples, 0);
            else
                msaaDS = nonMSAADS;

            if (shadowMap != null)
                shadowMap.Dispose();
            shadowMap = new RenderTarget2D(GraphicsDevice, (int)shadowMapSize, (int)shadowMapSize,
                                                1, SurfaceFormat.Single, RenderTargetUsage.PreserveContents);

            if (shadowMapDS != null)
                shadowMapDS.Dispose();
            shadowMapDS = new DepthStencilBuffer(GraphicsDevice, (int)shadowMapSize, (int)shadowMapSize, DepthFormat.Depth24Stencil8);
        }

        /// <summary>
        /// Handles input, and moves the point lights around
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Update(GameTime gameTime)
        {
            float dt = (float)gameTime.ElapsedGameTime.TotalSeconds;

            const float LightRotSpeed = 0.3f;

            Matrix rotMatrix = Matrix.CreateRotationY(LightRotSpeed * 2 * MathHelper.Pi * dt);
            foreach (Light light in lights)
            {
                if (light is PointLight)
                {
                    PointLight pointLight = (PointLight)light;
                    pointLight.Position = Vector3.Transform(pointLight.Position, rotMatrix);
                }
            }

            const float CamMoveSpeed = 1000.0f;
            const float CamRotSpeed = 0.3f;
			float slowdown = 1.0f;

            KeyboardState kbState = Keyboard.GetState();
			
			if (kbState.IsKeyDown(Keys.LeftShift))
				slowdown = 0.1f;

            if (kbState.IsKeyDown(Keys.W))
                camera.Position += camera.WorldMatrix.Forward * CamMoveSpeed * slowdown * dt;
            else if (kbState.IsKeyDown(Keys.S))
				camera.Position += camera.WorldMatrix.Backward * CamMoveSpeed * slowdown * dt;
            if (kbState.IsKeyDown(Keys.A))
				camera.Position += camera.WorldMatrix.Left * CamMoveSpeed * slowdown * dt;
            else if (kbState.IsKeyDown(Keys.D))
				camera.Position += camera.WorldMatrix.Right * CamMoveSpeed * slowdown * dt;
            if (kbState.IsKeyDown(Keys.Q))
				camera.Position += camera.WorldMatrix.Up * CamMoveSpeed * slowdown * dt;
            else if (kbState.IsKeyDown(Keys.E))
				camera.Position += camera.WorldMatrix.Down * CamMoveSpeed * slowdown * dt;

            if (kbState.IsKeyDown(Keys.Escape))
                Exit();

            if (kbState.IsKeyDown(Keys.M) && !lastKBState.IsKeyDown(Keys.M))
            {
                enableMSAA = !enableMSAA;
                CreateRenderTargets();
            }

            if (kbState.IsKeyDown(Keys.V) && !lastKBState.IsKeyDown(Keys.V))
            {
                graphics.SynchronizeWithVerticalRetrace = !graphics.SynchronizeWithVerticalRetrace;
                graphics.ApplyChanges();
            }

            if (kbState.IsKeyDown(Keys.G) && !lastKBState.IsKeyDown(Keys.G))
            {
                int size = (int)gBufferSize;
                size++;
                size = size % Enum.GetValues(typeof(GBufferSize)).Length;
                gBufferSize = (GBufferSize)size;
                CreateRenderTargets();
            }

            if (kbState.IsKeyDown(Keys.N) && !lastKBState.IsKeyDown(Keys.N))
                enableNormalMaps = !enableNormalMaps;

            if (kbState.IsKeyDown(Keys.P) && !lastKBState.IsKeyDown(Keys.P))
                enablePointLights = !enablePointLights;

            if (kbState.IsKeyDown(Keys.T) && !lastKBState.IsKeyDown(Keys.T))
            {
                int mode = (int)transparencyMode;
                mode++;
                mode = mode % Enum.GetValues(typeof(TransparencyMode)).Length;
                transparencyMode = (TransparencyMode)mode;
            }

            if (kbState.IsKeyDown(Keys.L) && !lastKBState.IsKeyDown(Keys.L))
            {
                if (shadowMapSize == ShadowMapSize.Size1024)
                    shadowMapSize = ShadowMapSize.Size2048;
                else
                    shadowMapSize = ShadowMapSize.Size1024;
                CreateRenderTargets();
            }

            MouseState mouseState = Mouse.GetState();
            int moveX = mouseState.X - lastMouseState.X;
            int moveY = mouseState.Y - lastMouseState.Y;

            if (mouseState.RightButton == ButtonState.Pressed)
            {
                camera.XRotation -= moveY * CamRotSpeed * dt;
                camera.YRotation -= moveX * CamRotSpeed * dt;
            }
       
            lastKBState = kbState;
            lastMouseState = mouseState;

            base.Update(gameTime);
        }

        /// <summary>
        /// Renders the scene
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
            // Calculate the FPS with filtering
            long currentTime = stopwatch.ElapsedMilliseconds;
            dtFilter[dtIndex % DTFilterSize] = currentTime - lastTime;
            long dtSum = 0;
            foreach (long dtVal in dtFilter)
                dtSum += dtVal;
            fps = (int)Math.Round(1000.0f / (dtSum / (float)DTFilterSize));
            lastTime = currentTime;
            dtIndex++;

            // Sort the transparents
            sortedTransparents.Clear();
            sortedTransparents.AddRange(transparents);
            sortedTransparents.Sort(CompareByDepth);

            // Go through our passes, one bye one
            DrawGBuffer();
            DrawLights();
            DrawCompositePass();

            if (transparencyMode == TransparencyMode.ForwardRendered)
                DrawTransparents();

            DrawScreen();
            DrawText();

            base.Draw(gameTime);
        }

        /// <summary>
        /// Passed as a delegate to List.Sort, to sort transparent instances by view-space depth
        /// </summary>
        private int CompareByDepth(ModelInstance a, ModelInstance b)
        {
            float aDepth = -Vector3.Transform(a.Position, camera.ViewMatrix).Z;
            float bDepth = -Vector3.Transform(b.Position, camera.ViewMatrix).Z;
            return (int)Math.Sign(aDepth - bDepth);
        }

        /// <summary>
        /// Draws all opaque instances to the G-Buffer, as well as transparents (if deferred mode
        /// is being used to render the transparents)
        /// </summary>
        private void DrawGBuffer()
        {
            PIXHelper.BeginEvent("Draw G-Buffer");

            GraphicsDevice.SetRenderTarget(0, depthIDBuffer);
            GraphicsDevice.SetRenderTarget(1, normalSpecularBuffer);
            GraphicsDevice.DepthStencilBuffer = nonMSAADS;
            GraphicsDevice.Clear(ClearOptions.DepthBuffer | ClearOptions.Stencil | ClearOptions.Target, Vector4.Zero, 1.0f, 0);

			PIXHelper.BeginEvent("Opaques");

            if (enableNormalMaps)
                gBufferEffect.CurrentTechnique = gBufferEffect.Techniques["GBufferNM"];
            else
                gBufferEffect.CurrentTechnique = gBufferEffect.Techniques["GBuffer"];

            // We'll make sure each instance has a unique ID by incrementing a variable
            // every time we render one.  0 is reserved for no instances, so we start
            // at one.
            int instanceID = 1;
            foreach (ModelInstance instance in opaques)
            {
                gBufferEffect.Parameters["InstanceID"].SetValue(instanceID);
                instance.Draw(GraphicsDevice, gBufferEffect, camera);
                ++instanceID;
            }

			PIXHelper.EndEvent();

            if (transparencyMode == TransparencyMode.ForwardRendered)
            {
                PIXHelper.EndEvent();
                return;
            }

			PIXHelper.BeginEvent("Transparents");

            if (enableNormalMaps)
                gBufferEffect.CurrentTechnique = gBufferEffect.Techniques["GBufferMaskNM"];
            else
                gBufferEffect.CurrentTechnique = gBufferEffect.Techniques["GBufferMask"];

            foreach (ModelInstance instance in sortedTransparents)
            {
                // For each instance, rotate through our 3 output masks
                gBufferEffect.Parameters["OutputMask"].SetValue(OutputMasks[instanceID % NumMasks]);
                gBufferEffect.Parameters["InstanceID"].SetValue(instanceID);                
                instance.Draw(GraphicsDevice, gBufferEffect, camera);
                ++instanceID;
            }


            PIXHelper.EndEvent();

			PIXHelper.EndEvent();
        }

        /// <summary>
        /// Renders the lighting pass by rendering all lights to the L-Buffer
        /// </summary>
        private void DrawLights()
        {
            PIXHelper.BeginEvent("Draw Lights");

            GraphicsDevice.SetRenderTarget(0, lightBuffer);
            GraphicsDevice.SetRenderTarget(1, null);
            GraphicsDevice.Clear(ClearOptions.Target, Vector4.Zero, 1.0f, 0);

            lightEffect.Parameters["FarClip"].SetValue(camera.FarClip);
            lightEffect.Parameters["GBufferDimensions"].SetValue(gBufferDimensions);
            lightEffect.Parameters["DepthIDTexture"].SetValue(depthIDBuffer.GetTexture());
            lightEffect.Parameters["NormalSpecTexture"].SetValue(normalSpecularBuffer.GetTexture());

            foreach (Light light in lights)
            {
                lightEffect.Parameters["LightColor"].SetValue(light.Color);

                if (light is PointLight && enablePointLights)
                {
                    PointLight pointLight = (PointLight)light;

                    Matrix worldMatrix = Matrix.CreateScale(pointLight.Range) * Matrix.CreateTranslation(pointLight.Position);
                    lightEffect.Parameters["WorldView"].SetValue(worldMatrix * camera.ViewMatrix);
                    lightEffect.Parameters["WorldViewProjection"].SetValue(worldMatrix * camera.ViewProjectionMatrix);
                    lightEffect.Parameters["LightRange"].SetValue(pointLight.Range);
                    lightEffect.Parameters["LightPosVS"].SetValue(Vector3.Transform(pointLight.Position, camera.ViewMatrix));

                    // For our light volume we'll want to render front faces if the volume intersects
                    // our far clipping plane, or back faces otherwise.
                    float lightDepth = -Vector3.Transform(pointLight.Position, camera.ViewMatrix).Z;
                    if (lightDepth > camera.FarClip - pointLight.Range)
                        lightEffect.CurrentTechnique = lightEffect.Techniques["PointLightFront"];
                    else
                        lightEffect.CurrentTechnique = lightEffect.Techniques["PointLightBack"];

                    lightEffect.Begin(SaveStateMode.None);
                    lightEffect.CurrentTechnique.Passes[0].Begin();

                    ModelMesh mesh = lightSphere.Meshes[0];
                    ModelMeshPart meshPart = mesh.MeshParts[0];

                    GraphicsDevice.Indices = mesh.IndexBuffer;
                    GraphicsDevice.VertexDeclaration = meshPart.VertexDeclaration;
                    GraphicsDevice.Vertices[0].SetSource(mesh.VertexBuffer, meshPart.StreamOffset, meshPart.VertexStride);

                    GraphicsDevice.DrawIndexedPrimitives(PrimitiveType.TriangleList,
                                                            meshPart.BaseVertex,
                                                            0,
                                                            meshPart.NumVertices,
                                                            meshPart.StartIndex,
                                                            meshPart.PrimitiveCount);

                    lightEffect.CurrentTechnique.Passes[0].End();
                    lightEffect.End();
                }
                else if (light is DirectionalLight)
                {
                    DirectionalLight directionalLight = (DirectionalLight)light;

                    // Do shadow mapping first
                    DrawShadowMap();

                    // Now draw the directional light
                    GraphicsDevice.SetRenderTarget(0, lightBuffer);
                    GraphicsDevice.DepthStencilBuffer = nonMSAADS;
                    lightEffect.Parameters["ShadowMapTexture"].SetValue(shadowMap.GetTexture());
                    lightEffect.Parameters["ViewToLight"].SetValue(camera.WorldMatrix * lightCam.ViewProjectionMatrix);
                    lightEffect.Parameters["ShadowMapDimensions"].SetValue(new Vector2((int)shadowMapSize));

                    Vector3 lightDirVS = Vector3.TransformNormal(directionalLight.Direction, camera.ViewMatrix);
                    lightEffect.Parameters["LightDirVS"].SetValue(Vector3.Normalize(lightDirVS));            

                    // Get the frustum corners for position reconstruction
                    Vector3[] cornersWS = camera.BoundingFrustum.GetCorners();
                    Vector3[] cornersVS = new Vector3[4];
                    for (int i = 0; i < 4; ++i)
                        cornersVS[i] = Vector3.Transform(cornersWS[i + 4], camera.ViewMatrix);
                    lightEffect.Parameters["FrustumCornersVS"].SetValue(cornersVS);

                    lightEffect.CurrentTechnique = lightEffect.Techniques["DirectionalLight"];

                    lightEffect.Begin(SaveStateMode.None);
                    lightEffect.CurrentTechnique.Passes[0].Begin();

                    quad.Draw(GraphicsDevice);

                    lightEffect.CurrentTechnique.Passes[0].End();
                    lightEffect.End();
                }
            }

            PIXHelper.EndEvent();
        }

        /// <summary>
        /// Renders the composite pass, where instances a re-rendered and sample the
        /// L-Buffer to determine the lighting value that should be applied.
        /// </summary>
        private void DrawCompositePass()
        {
            PIXHelper.BeginEvent("Draw Composite Pass");

            GraphicsDevice.SetRenderTarget(0, colorBuffer);
            GraphicsDevice.DepthStencilBuffer = msaaDS;
            GraphicsDevice.Clear(ClearOptions.Target|ClearOptions.DepthBuffer|ClearOptions.Stencil, 
                                        Color.Black, 1.0f, 0);

            compositeEffect.Parameters["GBufferDimensions"].SetValue(gBufferDimensions);
            compositeEffect.Parameters["RTDimensions"].SetValue(new Vector2(ScreenWidth, ScreenHeight));
            compositeEffect.Parameters["AmbientColor"].SetValue(AmbientLight);
            compositeEffect.Parameters["LightTexture"].SetValue(lightBuffer.GetTexture());
            compositeEffect.Parameters["DepthIDTexture"].SetValue(depthIDBuffer.GetTexture());
            compositeEffect.Parameters["Alpha"].SetValue(1.0f);

			PIXHelper.BeginEvent("Opaques");

            // Draw the opaques
            if (gBufferSize == GBufferSize.FullSize && transparencyMode == TransparencyMode.ForwardRendered)
                compositeEffect.CurrentTechnique = compositeEffect.Techniques["CompositeOpaque"];
            else
                compositeEffect.CurrentTechnique = compositeEffect.Techniques["CompositeOpaqueFiltered"];

            int instanceID = 1;
            foreach (ModelInstance instance in opaques)
            {
                compositeEffect.Parameters["InstanceID"].SetValue(instanceID);
                instance.Draw(GraphicsDevice, compositeEffect, camera);
                ++instanceID;
            }

			PIXHelper.EndEvent();

            if (transparencyMode == TransparencyMode.ForwardRendered)
            {
                PIXHelper.EndEvent();
                return;
            }

			PIXHelper.BeginEvent("Transparents");

            compositeEffect.CurrentTechnique = compositeEffect.Techniques["CompositeTransparent"];
            compositeEffect.Parameters["Alpha"].SetValue(0.5f);
            foreach (ModelInstance instance in sortedTransparents)
            {
                compositeEffect.Parameters["InstanceID"].SetValue(instanceID);
                compositeEffect.Parameters["XFilterOffsets"].SetValue(XFilterOffsets[instanceID % NumMasks]);
                compositeEffect.Parameters["YFilterOffsets"].SetValue(YFilterOffsets[instanceID % NumMasks]);
                instance.Draw(GraphicsDevice, compositeEffect, camera);
                ++instanceID;
            }

			PIXHelper.EndEvent();

            PIXHelper.EndEvent();
        }

        /// <summary>
        /// If regular foward rendering is being used for transparents, this method is called
        /// and renders the transparents
        /// </summary>
        private void DrawTransparents()
        {
            PIXHelper.BeginEvent("Draw Transparents Pass");

            List<Vector3> lightPositions = new List<Vector3>();
            List<float> lightRanges = new List<float>();
            List<Vector3> lightColors = new List<Vector3>();

            foreach (Light light in lights)
            {
                if (light is PointLight)
                {
                    PointLight pointLight = (PointLight)light;
                    lightPositions.Add(Vector3.Transform(pointLight.Position, camera.ViewMatrix));
                    lightRanges.Add(pointLight.Range);
                    lightColors.Add(pointLight.Color);
                }
                else if (light is DirectionalLight)
                {
                    DirectionalLight directionalLight = (DirectionalLight)light;

                    Vector3 lightDirVS = Vector3.TransformNormal(directionalLight.Direction, camera.ViewMatrix);
                    transparentEffect.Parameters["LightDirVS"].SetValue(Vector3.Normalize(lightDirVS));                    
                    transparentEffect.Parameters["LightColor"].SetValue(light.Color); 
                }
            }

            transparentEffect.Parameters["LightPositionsVS"].SetValue(lightPositions.ToArray());
            transparentEffect.Parameters["LightRanges"].SetValue(lightRanges.ToArray());
            transparentEffect.Parameters["LightColors"].SetValue(lightColors.ToArray());
            transparentEffect.Parameters["AmbientColor"].SetValue(AmbientLight);
            transparentEffect.Parameters["EnablePointLights"].SetValue(enablePointLights);
            transparentEffect.Parameters["ViewToLight"].SetValue(camera.WorldMatrix * lightCam.ViewProjectionMatrix);
            transparentEffect.Parameters["ShadowMapTexture"].SetValue(shadowMap.GetTexture());
            transparentEffect.Parameters["ShadowMapDimensions"].SetValue(new Vector2((int)shadowMapSize));

            if (enableNormalMaps)
                transparentEffect.CurrentTechnique = transparentEffect.Techniques["TransparentNM"];
            else
                transparentEffect.CurrentTechnique = transparentEffect.Techniques["Transparent"];

            foreach (ModelInstance instance in sortedTransparents)            
                instance.Draw(GraphicsDevice, transparentEffect, camera);

            PIXHelper.EndEvent();
        }

        private void DrawScreen()
        {
            PIXHelper.BeginEvent("Drawing Results to Screen");

            GraphicsDevice.SetRenderTarget(0, null);
            GraphicsDevice.DepthStencilBuffer = nonMSAADS;
            GraphicsDevice.Clear(ClearOptions.Target | ClearOptions.DepthBuffer | ClearOptions.Stencil, Vector4.Zero, 1.0f, 0);

            spriteBatch.Begin(SpriteBlendMode.None, SpriteSortMode.Immediate, SaveStateMode.None);
            spriteEffect.Begin(SaveStateMode.None);
            spriteEffect.CurrentTechnique.Passes[0].Begin();
            spriteBatch.Draw(colorBuffer.GetTexture(), Vector2.Zero, Color.White);
            spriteBatch.End();
            spriteEffect.CurrentTechnique.Passes[0].End();
            spriteEffect.End();
        }

        /// <summary>
        /// Draws the HUD
        /// </summary>
        private void DrawText()
        {
            int right = GraphicsDevice.Viewport.TitleSafeArea.Right;
            int bottom = GraphicsDevice.Viewport.TitleSafeArea.Bottom;
            spriteBatch.Begin(SpriteBlendMode.AlphaBlend, SpriteSortMode.Immediate, SaveStateMode.None);

            spriteBatch.DrawString(font, "FPS: " + fps, new Vector2(50, 50), Color.Yellow);

            int x = 10;
            int y = bottom - 175;
            spriteBatch.DrawString(font, "Controls:", new Vector2(x, y), Color.Yellow);
            spriteBatch.DrawString(font, "Camera Movement: W-A-S-D-F-Q-E", new Vector2(x + 20, y + 15), Color.Yellow);
            spriteBatch.DrawString(font, "Camera Rotation: Mouse + RB", new Vector2(x + 20, y + 30), Color.Yellow);
            spriteBatch.DrawString(font, "Change G-Buffer Size: G", new Vector2(x + 20, y + 45), Color.Yellow);
            spriteBatch.DrawString(font, "Toggle Normal Mapping: N", new Vector2(x + 20, y + 60), Color.Yellow);
            spriteBatch.DrawString(font, "Toggle MSAA: M", new Vector2(x + 20, y + 75), Color.Yellow);
            spriteBatch.DrawString(font, "Toggle VSYNC: V", new Vector2(x + 20, y + 90), Color.Yellow);
            spriteBatch.DrawString(font, "Switch Transparency Mode: T", new Vector2(x + 20, y + 105), Color.Yellow);
            spriteBatch.DrawString(font, "Toggle Point Lights: P", new Vector2(x + 20, y + 120), Color.Yellow);
            spriteBatch.DrawString(font, "Change Shadow Map Size: L", new Vector2(x + 20, y + 135), Color.Yellow);

            x = right - 400;
            y = bottom - 130;
            spriteBatch.DrawString(font, "G-Buffer Size: " + gBufferSize.ToString(), new Vector2(x, y), Color.Yellow);
            spriteBatch.DrawString(font, "Normal Mapping Enabled: " + enableNormalMaps, new Vector2(x, y + 15), Color.Yellow);
            spriteBatch.DrawString(font, "VSYNC Enabled: " + graphics.SynchronizeWithVerticalRetrace.ToString(), new Vector2(x, y + 30), Color.Yellow);
            spriteBatch.DrawString(font, "MSAA Enabled: " + enableMSAA.ToString(), new Vector2(x, y + 45), Color.Yellow);
            spriteBatch.DrawString(font, "Transparency Mode: " + transparencyMode.ToString(), new Vector2(x, y + 60), Color.Yellow);
            spriteBatch.DrawString(font, "Point Lights Enabled: " + enablePointLights.ToString(), new Vector2(x, y + 75), Color.Yellow);
            spriteBatch.DrawString(font, "Shadow Map Size: " + shadowMapSize.ToString(), new Vector2(x, y + 90), Color.Yellow);

            spriteBatch.End();
        }

        /// <summary>
        /// Draws all opaque instances to the shadow map
        /// </summary>
        private void DrawShadowMap()
        {
            PIXHelper.BeginEvent("Draw ShadowMap");            

            GraphicsDevice.SetRenderTarget(0, shadowMap);
            GraphicsDevice.DepthStencilBuffer = shadowMapDS;
            GraphicsDevice.Clear(ClearOptions.Target | ClearOptions.DepthBuffer | ClearOptions.Stencil, Vector4.One, 1.0f, 0);
            foreach (ModelInstance instance in opaques)
                instance.Draw(GraphicsDevice, shadowMapEffect, lightCam);

            PIXHelper.EndEvent();
        }

        private enum GBufferSize
        {
            FullSize = 0,
            HalfSize = 1,
            QuarterSize = 2
        }

        private enum TransparencyMode
        {
            ForwardRendered = 0,
            DeferredFourLayers = 1
        }

        private enum ShadowMapSize
        {
            Size1024 = 1024,
            Size2048 = 2048
        }        
    }
}
