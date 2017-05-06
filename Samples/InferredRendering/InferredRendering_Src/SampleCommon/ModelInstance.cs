//========================================================================
//
//	Common Sample Framework
//
//		by MJP  (mpettineo@gmail.com)
//		12/14/08      
//
//========================================================================
//
//	File:		ModelInstance.cs
//
//	Desc:		Represents a single instance of a Model.
//
//========================================================================

using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace SampleCommon
{

    public class ModelInstance
    {
        Model model;
        Matrix worldMatrix = Matrix.Identity;
        Matrix[] bones;
        Vector3 specularAlbedo;
        float specularExponent;
        Texture2D diffuseMap;
        Texture2D normalMap;
        Vector2 texScale;

        /// <summary>
        /// Gets or sets the world matrix for this instance
        /// </summary>
        public Matrix WorldMatrix
        {
            get { return worldMatrix; }
            set { worldMatrix = value; }
        }

        /// <summary>
        /// Gets or sets the position for this instance
        /// </summary>
        public Vector3 Position
        {
            get { return worldMatrix.Translation; }
            set { worldMatrix.Translation = value; }
        }

        /// <summary>
        /// Gets or sets the rotation for this instance
        /// </summary>
        public Quaternion Rotation
        {
            get { return Quaternion.CreateFromRotationMatrix(worldMatrix); }
            set
            {
                Vector3 position = worldMatrix.Translation;
                worldMatrix = Matrix.CreateFromQuaternion(value);
                worldMatrix.Translation = position;
            }
        }

        /// <summary>
        /// Gets or sets the rotation for this instance, as a matrix
        /// </summary>
        public Matrix RotationMatrix
        {
            get
            {
                Matrix rotationMatrix = worldMatrix;
                rotationMatrix.Translation = Vector3.Zero;
                return rotationMatrix;
            }
            set
            {
                Vector3 position = worldMatrix.Translation;
                worldMatrix = value;
                worldMatrix.Translation = position;
            }
        }

        public Texture2D NormalMap
        {
            get { return normalMap; }
        }

        public Texture2D DiffuseMap
        {
            get { return diffuseMap; }
        }

        public float SpecularExponent
        {
            get { return specularExponent; }
        }

        public Vector3 SpecularAlbedo
        {
            get { return specularAlbedo; }
        }

        public Model Model
        {
            get { return model; }
        }

        /// <summary>
        /// Creates an instance of the specified model
        /// </summary>
        /// <param name="model">The model used by this instance</param>
        public ModelInstance(Model model, 
                                Texture2D diffuseMap, 
                                Texture2D normalMap,
                                float specularExponent,
                                Vector3 specularAlbedo,
                                Vector2 texScale)
        {
            this.model = model;
            bones = new Matrix[model.Bones.Count];
            model.CopyAbsoluteBoneTransformsTo(bones);
            this.diffuseMap = diffuseMap;
            this.normalMap = normalMap;
            this.specularExponent = specularExponent;
            this.specularAlbedo = specularAlbedo;
            this.texScale = texScale;
        }

        /// <summary>
        /// Draws the model, and sets the world matrix parameter
        /// of the specified Effect
        /// </summary>
        /// <param name="graphicsDevice">The GraphicsDevice to use for drawing</param>
        /// <param name="effect">Sets the world matrix parameter for this Effect</param>
        /// <param name="camera">The camera from which view and projection matrices will be retrieved</param>
        public void Draw(GraphicsDevice graphicsDevice, Effect effect, Camera camera)
        {         
            EffectParameter param = effect.Parameters["View"];
            if (param != null)
                param.SetValue(camera.ViewMatrix);

            param = effect.Parameters["Projection"];
            if (param!= null)
                param.SetValue(camera.ProjectionMatrix);

            param = effect.Parameters["ViewProjection"];
            if (param != null)
                param.SetValue(camera.ViewProjectionMatrix);

            param = effect.Parameters["FarClip"];
            if (param != null)
                param.SetValue(camera.FarClip);

            param = effect.Parameters["DiffuseMap"];
            if (param != null)
                param.SetValue(diffuseMap);

            param = effect.Parameters["NormalMap"];
            if (param != null)
                param.SetValue(normalMap);

            param = effect.Parameters["SpecularExponent"];
            if (param != null)
                param.SetValue(specularExponent);

            param = effect.Parameters["SpecularAlbedo"];
            if (param != null)
                param.SetValue(specularAlbedo);

            param = effect.Parameters["TexScale"];
            if (param != null)
                param.SetValue(texScale);

            for (int i = 0; i < model.Meshes.Count; i++)
            {
                ModelMesh mesh = model.Meshes[i];

                Matrix transform;
                Matrix.Multiply(ref bones[mesh.ParentBone.Index], ref worldMatrix, out transform);
                param = effect.Parameters["World"];
                if (param != null)
                    param.SetValue(transform);

                Matrix transpose, inverseTranspose;
                Matrix.Transpose(ref transform, out transpose);
                Matrix.Invert(ref transpose, out inverseTranspose);
                param = effect.Parameters["WorldIT"];
                if (param != null)
                    param.SetValue(inverseTranspose);

                param = effect.Parameters["WorldView"];
                if (param != null)
                    param.SetValue(transform * camera.ViewMatrix);

                param = effect.Parameters["WorldViewProjection"];
                if (param != null)
                    param.SetValue(transform * camera.ViewProjectionMatrix);

                graphicsDevice.Indices = mesh.IndexBuffer;

                for (int j = 0; j < mesh.MeshParts.Count; j++)
                {
                    ModelMeshPart meshPart = mesh.MeshParts[j];

                    effect.Begin(SaveStateMode.None);

                    foreach (EffectPass pass in effect.CurrentTechnique.Passes)
                    {
                        pass.Begin();

                        graphicsDevice.VertexDeclaration = meshPart.VertexDeclaration;
                        graphicsDevice.Vertices[0].SetSource(mesh.VertexBuffer, meshPart.StreamOffset, meshPart.VertexStride);

                        graphicsDevice.DrawIndexedPrimitives(PrimitiveType.TriangleList,
                                                                meshPart.BaseVertex,
                                                                0,
                                                                meshPart.NumVertices,
                                                                meshPart.StartIndex,
                                                                meshPart.PrimitiveCount);

                        pass.End();
                    }

                    effect.End();
                }
            }

            graphicsDevice.Vertices[0].SetSource(null, 0, 0);
            graphicsDevice.VertexDeclaration = null;
            graphicsDevice.Indices = null;
        }
    }
}