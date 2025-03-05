#include "QtMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>

#include <QFileInfo>
#include <QDir>
#include <QImage>
#include <QQueue>

QMatrix4x4 converter(aiMatrix4x4 const& aiMat4)
{
    QMatrix4x4 mat4;
    ai_real const* dataPtr = (ai_real const*)&aiMat4;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            mat4(i, j) = *(dataPtr + (4 * i) + j);
    return mat4;
}

QVector3D converter(aiVector3D const& aiVec3)
{
    return QVector3D(aiVec3.x, aiVec3.y, aiVec3.z);
}

QVector<QSharedPointer<QtMesh::Material>> LoadMaterialsFromScene(aiScene const* scene, QString model_path)
{
    static QStringList TextureNameMap = {
        "None",      "Diffuse",      "Specular",      "Ambient",      "Emissive",  "Height",
        "Normal",    "Shininess",    "Opacity",       "Displacement", "LightMap",  "Reflection",
        "BaseColor", "NormalCamera", "EmissionColor", "Metallic",     "Roughness", "AmbientOcclusion",
        "Unknown",   "Sheen",        "ClearCoat",     "Transmission"};

    QDir model_dir = QFileInfo(model_path).dir();
    QVector<QSharedPointer<QtMesh::Material>> materials;
    for (unsigned int m = 0; m < scene->mNumMaterials; m++)
    {
        QSharedPointer<QtMesh::Material> material = QSharedPointer<QtMesh::Material>::create();
        aiMaterial const* amterial = scene->mMaterials[m];
        for (int i = aiTextureType_DIFFUSE; i < AI_TEXTURE_TYPE_MAX; i++)
        {
            if (i == aiTextureType_UNKNOWN) continue;
            auto count = amterial->GetTextureCount(aiTextureType(i));
            for (unsigned int j = 0; j < count; j++)
            {
                aiString path;
                amterial->GetTexture(aiTextureType(i), j, &path);
                QString name = amterial->GetName().C_Str();
                if (name.startsWith('/')) model_dir.setPath(model_dir.filePath(name.mid(1, name.lastIndexOf('/') - 1)));
                QString realPath = model_dir.filePath(path.C_Str());
                QImage image;
                if (QFile::exists(realPath))
                    image.load(realPath);
                else
                {
                    aiTexture const* embTexture = scene->GetEmbeddedTexture(path.C_Str());
                    if (embTexture != nullptr)
                    {
                        if (embTexture->mHeight == 0)
                            image.loadFromData((uchar*)embTexture->pcData, embTexture->mWidth,
                                               embTexture->achFormatHint);
                        else
                            image = QImage((uchar*)embTexture->pcData, embTexture->mWidth, embTexture->mHeight,
                                           QImage::Format_ARGB32);
                    }
                }
                QString texture_name = TextureNameMap[i];
                if (j != 0) texture_name += QString::number(j);
                if (!image.isNull()) material->properties[texture_name] = image;
            }
        }
        materials << material;
    }
    return materials;
}

QSharedPointer<QtMesh> QtMesh::Load(QString const& path)
{
    QSharedPointer<QtMesh> qmesh;

    Assimp::Importer importer;
    aiScene const* scene =
        importer.ReadFile(path.toUtf8().constData(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                                                         aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes);
    if (!scene) return qmesh;

    qmesh = QSharedPointer<QtMesh>::create();
    qmesh->m_materials = LoadMaterialsFromScene(scene, path);
    float x_min = std::numeric_limits<float>::max(), x_max = std::numeric_limits<float>::min(),
          y_min = std::numeric_limits<float>::max(), y_max = std::numeric_limits<float>::min(),
          z_min = std::numeric_limits<float>::max(), z_max = std::numeric_limits<float>::min();
    QQueue<QPair<aiNode*, aiMatrix4x4>> qnode;
    qnode.push_back({scene->mRootNode, aiMatrix4x4()});
    while (!qnode.isEmpty())
    {
        QPair<aiNode*, aiMatrix4x4> node = qnode.takeFirst();
        for (unsigned int i = 0; i < node.first->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node.first->mMeshes[i]];
            x_min = std::min(x_min, mesh->mAABB.mMin.x);
            y_min = std::min(y_min, mesh->mAABB.mMin.y);
            z_min = std::min(z_min, mesh->mAABB.mMin.z);
            x_max = std::max(x_max, mesh->mAABB.mMax.x);
            y_max = std::max(y_max, mesh->mAABB.mMax.y);
            z_max = std::max(z_max, mesh->mAABB.mMax.z);
            SubMeshData qsub_mesh;
            qsub_mesh.vertices_offset = qmesh->m_vertices.size();
            qsub_mesh.vertices_num = mesh->mNumVertices;
            qsub_mesh.material_index = mesh->mMaterialIndex;
            qsub_mesh.local_transfrom = converter(node.second);
            qmesh->m_vertices.resize(qsub_mesh.vertices_offset + qsub_mesh.vertices_num);
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                QtMesh::Vertex& vertex = qmesh->m_vertices[qsub_mesh.vertices_offset + i];
                vertex.position = converter(mesh->mVertices[i]);
                if (mesh->mNormals) vertex.normal = converter(mesh->mNormals[i]);
                if (mesh->mTextureCoords[0])
                {
                    vertex.uv.setX(mesh->mTextureCoords[0][i].x);
                    vertex.uv.setY(mesh->mTextureCoords[0][i].y);
                }
                if (mesh->mTangents) vertex.tangent = converter(mesh->mTangents[i]);
                if (mesh->mBitangents) vertex.bitangent = converter(mesh->mBitangents[i]);
            }
            qsub_mesh.indices_offset = qmesh->m_indices.size();
            qsub_mesh.indices_num = 0;
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                {
                    qmesh->m_indices.push_back(face.mIndices[j]);
                    qsub_mesh.indices_num++;
                }
            }
            qmesh->m_submeshes << qsub_mesh;
        }
        for (unsigned int i = 0; i < node.first->mNumChildren; i++)
            qnode.push_back({node.first->mChildren[i], node.second * node.first->mChildren[i]->mTransformation});
    }
    qmesh->m_extents[0] = x_min;
    qmesh->m_extents[1] = y_min;
    qmesh->m_extents[2] = z_min;
    qmesh->m_extents[3] = x_max;
    qmesh->m_extents[4] = y_max;
    qmesh->m_extents[5] = z_max;
    //qDebug() << x_min << y_min << z_min << x_max << y_max << z_max;

    return qmesh;
}