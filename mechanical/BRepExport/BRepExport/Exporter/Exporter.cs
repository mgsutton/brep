using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Ansys.ACT.Interfaces.Geometry;
using Ansys.ACT.Interfaces.Mechanical;
using Ansys.UI.Toolkit;
using g3;
using Google.Protobuf;
using Padt.Brep.Proto;
using PADT.BRepExport.Core;

namespace PADT.BRepExport
{
  public static class Exporter
  {
    private class Implemenation
    {
      public IMechanicalExtAPI ExtAPI { get; set; }
      
      public void Write(string outputPath)
      {
        var gd = ExtAPI.DataModel.GeoData;
        List<BRepEntity> entities = new List<BRepEntity>();
        int assemblyId = 1;
        foreach (var assembly in gd.Assemblies)
        {
          entities.AddRange(BuildAssemblyMessages(assembly, assemblyId++));
        }
        try
        {
          using(FileStream fs = new FileStream(outputPath, FileMode.Create))
          {
            foreach(var entity in entities)
            {
              entity.WriteDelimitedTo(fs);
            }
          }
        } catch(IOException e)
        {
          ExtAPI.Log.WriteError("Cannot export BRep. Reason:");
          ExtAPI.Log.WriteError(e.Message);
        }        
      }

      private List<BRepEntity>
        BuildAssemblyMessages(IGeoAssembly assembly, int id)
      {
        List<BRepEntity> entities = new List<BRepEntity>();
        var protoAssembly = new Assembly
        {
          Id = id,
          Source = ""
        };
        var partIds = (from p in assembly.Parts
                      select (long)p.Id).ToList();
        protoAssembly.Parts.AddRange(partIds);
        entities.Add(new BRepEntity
        {
          Assembly = protoAssembly
        });
        foreach (var part in assembly.Parts)
        {
          entities.AddRange(BuildPartMessages(part));
        }
        return entities;
      }

      private List<BRepEntity>
        BuildPartMessages(IGeoPart part)
      {
        List<BRepEntity> entities = new List<BRepEntity>();
        var protoPart = new Part
        {
          Id = part.Id
        };
        var bodyIds = (from b in part.Bodies
                       select (long)b.Id).ToList();
        protoPart.Bodies.AddRange(bodyIds);
        entities.Add(new BRepEntity
        {
          Part = protoPart
        });

        foreach(var body in part.Bodies)
        {
          entities.AddRange(BuildBodyMessages(body as IGeoBody));
        }
        return entities;
      }

      private List<BRepEntity>
        BuildBodyMessages(IGeoBody body)
      {
        List<BRepEntity> entities = new List<BRepEntity>();
        var protoBody = new Body
        {
          Id = body.Id
        };
        var faceIds = (from f in body.Faces
                       select (long)f.Id).ToList();
        protoBody.Faces.AddRange(faceIds);
        foreach(var s in body.Shells)
        {
          var protoShell = new Shell();
          var shellFaceIds = (from f in s.Faces
                              select (long)f.Id).ToList();
          protoShell.Faces.AddRange(shellFaceIds);
          protoBody.Shells.Add(protoShell);
        }
        entities.Add(new BRepEntity
        {
          Body = protoBody
        });
        foreach(var f in body.Faces)
        {
          entities.AddRange(BuildFaceMessages(f as IGeoFace));
        }
        foreach(var e in body.Edges)
        {
          entities.AddRange(BuildEdgeMessages(e as IGeoEdge));
        }
        foreach(var v in body.Vertices)
        {
          entities.AddRange(BuildVertexMessages(v as IGeoVertex));
        }
        return entities;
      }

      private List<BRepEntity>
        BuildFaceMessages(IGeoFace face)
      {
        List<BRepEntity> entities = new List<BRepEntity>();
        var protoSurface = new Surface();
        Vector3d[] p = BufferUtil.ToVector3d(face.Points);
        Vector3d[] n = BufferUtil.ToVector3d(face.Normals);
        foreach (var v in p)
        {
          var protoPoint = new Vector3
          {
            X = v.x,
            Y = v.y,
            Z = v.z
          };
          var uv = face.ParamAtPoint(new double[]{ v.x,v.y,v.z});
          var protoUV = new Vector2
          {
            U = uv[0],
            V = uv[1]
          };
          protoSurface.Points.Add(protoPoint);
          protoSurface.Parameters.Add(protoUV);
        }
        foreach (var v in n)
        {
          var protoNormal = new Vector3
          {
            X = v.x,
            Y = v.y,
            Z = v.z
          };
          protoSurface.Normals.Add(protoNormal);
        }
        // Get the indices into the proper form
        int i = 0;
        while (i < face.Indices.Length)
        {
          if (face.Indices[i] == 3)
          {
            int iv0 = face.Indices[++i];
            int iv1 = face.Indices[++i];
            int iv2 = face.Indices[++i];
            Vector3d V0, V1, V2, N0;
            V0 = p[iv0];
            V1 = p[iv1];
            V2 = p[iv2];
            N0 = n[iv0];
            Vector3d edge1 = V1 - V0;
            Vector3d edge2 = V2 - V0;
            Vector3d normal = edge1.Cross(edge2);
            if (normal.Dot(N0) > 0)
            {
              protoSurface.Triangles.Add(new Facet
              {
                I = iv0,
                J = iv1,
                K = iv2
              });
            }
            else
            {
              protoSurface.Triangles.Add(new Facet
              {
                I = iv0,
                J = iv2,
                K = iv1
              });
            }
          }
          else
          {
            throw new InvalidDataException("Cannot convert an ANSYS facet " +
              "list with a facet containing more than 3 vertices.");
          }
          i++;
        }
               

        var protoFace = new Face
        {
          Id = face.Id,
          Surface = protoSurface
        };

        var edges = (from e in face.Edges
                     select (long)e.Id).ToList();
        protoFace.Edges.AddRange(edges);

        foreach(var l in face.Loops)
        {
          var protoLoop = new Loop();
          var loopEdges = (from e in l.Edges
                           select (long)e.Id).ToList();
          protoLoop.Edges.AddRange(loopEdges);
          protoFace.Loops.Add(protoLoop);
        }
        entities.Add(new BRepEntity
        {
          Face = protoFace
        });
        return entities;
      }


      private List<BRepEntity> 
        BuildEdgeMessages(IGeoEdge edge)
      {
        List<BRepEntity> entities = new List<BRepEntity>();
        var protoCurve = new Curve();
        foreach (var triple in edge.Points.SplitToArray(3))
        {
          var protoPoint = new Vector3
          {
            X = triple[0],
            Y = triple[1],
            Z = triple[2]
          };
          double s = edge.ParamAtPoint(triple);
          
          protoCurve.Points.Add(protoPoint);
          protoCurve.Parameters.Add(s);
        }
        protoCurve.IsParamReversed = edge.IsParamReversed;
        var protoEdge = new Edge
        {
          Curve = protoCurve,
          Id = edge.Id,
          Start = edge.StartVertex == null ? -1 : edge.StartVertex.Id,
          End = edge.EndVertex == null ? -1: edge.EndVertex.Id
        };
        entities.Add(new BRepEntity
        {
          Edge = protoEdge
        });
        return entities;
      }

      private List<BRepEntity>
        BuildVertexMessages(IGeoVertex vertex)
      {
        List<BRepEntity> entities = new List<BRepEntity>();
        var protoPoint = new Vector3
        {
          X = vertex.X,
          Y = vertex.Y,
          Z = vertex.Z
        };
        Vertex protoVertex = new Vertex
        {
          Id = vertex.Id,
          Point = protoPoint,
        };
        entities.Add(new BRepEntity
        {
          Vertex = protoVertex
        });
        return entities;
      }
    }


    public static void Export(IMechanicalExtAPI ExtAPI)
    {
      string directory = "";
      string name = "";
      string selectedFile;
      int filterIndex;
      Window mainWin = (ExtAPI.UserInterface.MainWindow as Window);
      var result = FileDialog.ShowFileDialog(
          mainWin,
          true,
          directory,
          "BRep files (*.brep)|*.brep",
          0,
          out selectedFile,
          out filterIndex,
          "Export BRep File", name,
          true);
      if (result == DialogResult.OK)
      {
        // Write the data
        Implemenation writer = new Implemenation { ExtAPI = ExtAPI };
        writer.Write(selectedFile);
        Core.RecentFolderDataStore.StoreRecentFolder(
            "BRepExporter",
            "ExportFileLocation",
            Directory.GetParent(selectedFile).FullName);

      }

     
    }
  }
}

