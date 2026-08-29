"""Interior PBR preview — renders each SM_Int_* from an interior viewpoint
with its Int_* emissive lighting so the walkable space reads clearly."""
import os, math
import numpy as np
from PIL import Image

OBJDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\obj"
TEXDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\Textures"
OUTDIR = r"C:\Users\akuma\Adastrea\Assets\FBX\generated\previews"

INTERIORS = {
    "SM_Int_Fighter_Cockpit":        ("T_Int_Cockpit", "T_Int_Cockpit"),
    "SM_Int_Freighter_CrewQuarters": ("T_Int_Wall", "T_Int_Glow"),
    "SM_Int_Generationship_Hab":     ("T_Int_Wall", "T_Int_Glow"),
    "SM_Int_Standard_Corridor":      ("T_Int_Wall", "T_Int_Glow"),
    "SM_Int_Freighter_EngineRoom":   ("T_Int_Wall", "T_Int_Glow"),
    "SM_Int_Standard_Airlock":       ("T_Int_Wall", "T_Int_Glow"),
}
# auto-add any SM_Int_* OBJ found on disk (so future interiors render w/o editing)
if os.path.isdir(OBJDIR):
    for f in sorted(os.listdir(OBJDIR)):
        if f.startswith("SM_Int_") and f.endswith(".obj"):
            n = f[:-4]
            if n not in INTERIORS:
                INTERIORS[n] = ("T_Int_Wall", "T_Int_Glow")

def load_obj(path):
    verts=[]; uvs=[]; faces=[]
    with open(path) as f:
        for line in f:
            p=line.split()
            if not p: continue
            if p[0]=='v': verts.append((float(p[1]),float(p[2]),float(p[3])))
            elif p[0]=='vt': uvs.append((float(p[1]),float(p[2])))
            elif p[0]=='f':
                tri=[t.split('/') for t in p[1:]]
                fc=[]
                for vi,uv,_ in tri: fc.append((int(vi)-1, int(uv)-1 if uv else -1))
                for k in range(1,len(fc)-1): faces.append((fc[0],fc[k],fc[k+1]))
    return np.array(verts,float), np.array(uvs,float), np.array(faces,int)

def load_tex(name):
    D=np.asarray(Image.open(os.path.join(TEXDIR,name+"_D.png")).convert('RGB'),float)/255.0
    E=np.asarray(Image.open(os.path.join(TEXDIR,name+"_E.png")).convert('RGB'),float)/255.0
    return D,E

def render(name, wall_tex, glow_tex, W=1000, H=760):
    V,U,F = load_obj(os.path.join(OBJDIR,name+".obj"))
    # Use a fixed isometric-style orthographic projection (same as render_objs.py)
    # so the camera is never trapped inside the shell — renders reliably for all assets.
    az,el = math.radians(55), math.radians(22)
    ca,sa,ce,se = math.cos(az),math.sin(az),math.cos(el),math.sin(el)
    x,y,z = V[:,0],V[:,1],V[:,2]
    x1 = x*ca - y*sa
    y1 = x*sa + y*ca
    y2 = y1*ce - z*se
    pts2 = np.stack([x1, y2], axis=1)   # (P,2)
    # orthographic scale to frame
    minp=pts2.min(0); maxp=pts2.max(0)
    span=maxp-minp
    scale=min((W-100)/max(span[0],1e-6),(H-100)/max(span[1],1e-6))
    pts2=(pts2-minp)*scale
    # center the content bounding box in the frame
    w2=pts2[:,0].max()-pts2[:,0].min(); h2=pts2[:,1].max()-pts2[:,1].min()
    shift=np.array([(W-w2)/2 - pts2[:,0].min(), (H-h2)/2 - pts2[:,1].min()])
    pts2=pts2+shift
    Vc = V - V.mean(0)   # view vectors (for normal shading depth loosely)
    Ldir=np.array([0.3,-0.6,0.8]); Ldir/=np.linalg.norm(Ldir)
    amb=0.4; dif=0.7
    cam = V.mean(0) + np.array([5,5,20])   # light-ish reference for normals
    wallD,wallE=load_tex(wall_tex)
    glowD,glowE=load_tex(glow_tex)
    fb=np.ones((H,W,3),float)*0.01
    zb=np.full((H,W),1e9)
    for (a,b,c) in F:
        i0,u0f=a; i1,u1f=b; i2,u2f=c
        p0,p1,p2=pts2[i0],pts2[i1],pts2[i2]
        e1=p1-p0; e2=p2-p0
        if e1[0]*e2[1]-e1[1]*e2[0]>=0: continue
        w0,w1,w2=V[i0],V[i1],V[i2]
        n=np.cross(w1-w0,w2-w0); nl=np.linalg.norm(n)
        if nl<1e-9: continue
        n=n/nl
        if np.dot(n,(cam-w0))<0: n=-n
        lam=amb+dif*max(np.dot(n,Ldir),0)
        area=e1[0]*e2[1]-e1[1]*e2[0]
        if abs(area)<1e-9: continue
        x0=max(0,int(min(p0[0],p1[0],p2[0]))); x1=min(W-1,int(max(p0[0],p1[0],p2[0])))
        y0=max(0,int(min(p0[1],p1[1],p2[1]))); y1=min(H-1,int(max(p0[1],p1[1],p2[1])))
        got_uv = u0f>=0 and u1f>=0 and u2f>=0
        if got_uv: u0,u1,u2=U[u0f],U[u1f],U[u2f]
        # pick diffuse from wall, emissive from glow (resized to match)
        D=wallD
        E=glowE
        th,tw=D.shape[0],D.shape[1]
        # ensure glow matches wall dims for consistent indexing
        if E.shape != D.shape:
            sy=th/float(E.shape[0]); sx=tw/float(E.shape[1])
            ry=(np.arange(th)/sy).astype(int).clip(0,E.shape[0]-1)
            rx=(np.arange(tw)/sx).astype(int).clip(0,E.shape[1]-1)
            E=E[np.ix_(ry,rx)]
        for yy in range(y0,y1+1):
            for xx in range(x0,x1+1):
                aa=((p1[1]-p2[1])*(xx-p2[0])+(p2[0]-p1[0])*(yy-p2[1]))/area
                bb=((p2[1]-p0[1])*(xx-p2[0])+(p0[0]-p2[0])*(yy-p2[1]))/area
                cc=1-aa-bb
                if aa<0 or bb<0 or cc<0: continue
                zview=aa*(-Vc[i0,2])+bb*(-Vc[i1,2])+cc*(-Vc[i2,2])
                if zview>zb[yy,xx]: continue
                zb[yy,xx]=zview
                if got_uv:
                    uu=u0[0]*aa+u1[0]*bb+u2[0]*cc; vv=u0[1]*aa+u1[1]*bb+u2[1]*cc
                    tx=int((uu%1.0)*tw)%tw; ty=int((vv%1.0)*th)%th
                    col=wallD[ty,tx]
                    em=E[ty,tx]
                    emag=em.max()
                    if emag>0.25:
                        col=col*lam*0.5+em*np.array([1.8,1.8,2.0])
                    else:
                        col=col*lam
                else:
                    col=np.array([0.5,0.52,0.55])*lam
                fb[yy,xx]=np.clip(col,0,1)
    out=os.path.join(OUTDIR,name+"_interior.png")
    Image.fromarray((fb*255).astype(np.uint8)).save(out)
    print("rendered", os.path.basename(out))

for name,(w,g) in INTERIORS.items():
    try:
        render(name,w,g)
    except Exception as e:
        print("ERR",name,repr(e))