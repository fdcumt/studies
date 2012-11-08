// RenderToon.cpp
//

#include "StdAfx.h"
#include "Toon.h"
#include "RenderToon.h"

////////////////////////////////////////////////////////////////////////////////
// class BasicScene
////////////////////////////////////////////////////////////////////////////////

LPCTSTR BasicScene::TextureFile = _T("desert.bmp");

BOOL BasicScene::Init() {
    IDirect3DVertexBuffer9* floor;
    m_D3DDev->CreateVertexBuffer(6 * sizeof(Vertex), 0, Vertex::FVF, D3DPOOL_MANAGED, &floor, NULL);
    m_Floor.Attach(floor);

    Vertex* v = 0;
    floor->Lock(0, 0, (void**) &v, 0);

    v[0] = Vertex(-20.0f, -2.5f, -20.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    v[1] = Vertex(-20.0f, -2.5f,  20.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    v[2] = Vertex( 20.0f, -2.5f,  20.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);

    v[3] = Vertex(-20.0f, -2.5f, -20.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    v[4] = Vertex( 20.0f, -2.5f,  20.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
    v[5] = Vertex( 20.0f, -2.5f, -20.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);

    floor->Unlock();

    ID3DXMesh* pillar;
    D3DXCreateCylinder(m_D3DDev, 0.5f, 0.5f, 5.0f, 20, 20, &pillar, NULL);
    m_Pillar.Attach(pillar);

    IDirect3DTexture9* tex;
    HRESULT hr = D3DXCreateTextureFromFile(m_D3DDev, TextureFile, &tex);
    SGL_FAILED_DO(hr, MYTRACE_DX("D3DXCreateTextureFromFile", hr); return FALSE);
    m_FloorTex.Attach(tex);

    // Pre-Render Setup
    m_D3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    m_D3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    m_D3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

    D3DXVECTOR3 dir(0.707f, -0.707f, 0.707f);
    D3DXCOLOR clr(1.0f, 1.0f, 1.0f, 1.0f);
    D3DLIGHT9 light;
    SGL::InitDirLight(&light, dir, clr);

    m_D3DDev->SetLight(0, &light);
    m_D3DDev->LightEnable(0, TRUE);
    m_D3DDev->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
    m_D3DDev->SetRenderState(D3DRS_SPECULARENABLE, TRUE);

    return TRUE;
}

void BasicScene::Draw(float scale) {
    // Render
    D3DXMATRIX T, R, P, S;

    D3DXMatrixScaling(&S, scale, scale, scale);

    // used to rotate cylinders to be parallel with world's y-axis
    D3DXMatrixRotationX(&R, -D3DX_PI * 0.5f);

    // draw floor
    D3DXMatrixIdentity(&T);
    T = T * S;
    m_D3DDev->SetTransform(D3DTS_WORLD, &T);
    m_D3DDev->SetMaterial(&SGL::WHITE_MTRL);
    m_D3DDev->SetTexture(0, m_FloorTex);
    m_D3DDev->SetStreamSource(0, m_Floor, 0, sizeof(Vertex));
    m_D3DDev->SetFVF(Vertex::FVF);
    m_D3DDev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

    // draw pillars
    m_D3DDev->SetMaterial(&SGL::BLUE_MTRL);
    m_D3DDev->SetTexture(0, NULL);
    for (int i = 0; i < 5; ++i) {
        D3DXMatrixTranslation(&T, -5.0f, 0.0f, -15.0f + (i * 7.5f));
        P = R * T * S;
        m_D3DDev->SetTransform(D3DTS_WORLD, &P);
        m_Pillar->DrawSubset(0);

        D3DXMatrixTranslation(&T, 5.0f, 0.0f, -15.0f + (i * 7.5f));
        P = R * T * S;
        m_D3DDev->SetTransform(D3DTS_WORLD, &P);
        m_Pillar->DrawSubset(0);
    }
}

////////////////////////////////////////////////////////////////////////////////
// 渲染类 RenderToon
////////////////////////////////////////////////////////////////////////////////

LPCTSTR RenderToon::MeshDumpFile = _T("MeshDump.txt");
LPCTSTR RenderToon::VertexShaderFile = _T("toon.txt");
LPCTSTR RenderToon::ToonShadeTexFile = _T("toonshade.bmp");

BOOL RenderToon::Init(UINT width, UINT height, HWND hwnd, BOOL windowed, D3DDEVTYPE devType) {
    HRESULT hr = Render::Init(width, height, hwnd, windowed, devType);
    SGL_FAILED_DO(hr, MYTRACE_DX("Render::Init", hr); return FALSE);

    // Create geometry and compute corresponding world matrix and color for each mesh.
    ID3DXMesh* mesh;
    D3DXCreateTeapot(m_D3DDev, &mesh, NULL);
    m_Meshes[0].Attach(mesh);
    D3DXCreateSphere(m_D3DDev, 1.0f, 20, 20, &mesh, NULL);
    m_Meshes[1].Attach(mesh);
    D3DXCreateTorus(m_D3DDev, 0.5f, 1.0f, 20, 20, &mesh, NULL);
    m_Meshes[2].Attach(mesh);
    D3DXCreateCylinder(m_D3DDev, 0.5f, 0.5f, 2.0f, 20, 20, &mesh, NULL);
    m_Meshes[3].Attach(mesh);

    D3DXMatrixTranslation(&m_WorldMatrices[0],  0.0f,  2.0f, 0.0f);
    D3DXMatrixTranslation(&m_WorldMatrices[1],  0.0f, -2.0f, 0.0f);
    D3DXMatrixTranslation(&m_WorldMatrices[2], -3.0f,  0.0f, 0.0f);
    D3DXMatrixTranslation(&m_WorldMatrices[3],  3.0f,  0.0f, 0.0f);

    m_MeshColors[0] = D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f);
    m_MeshColors[1] = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
    m_MeshColors[2] = D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f);
    m_MeshColors[3] = D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f);

    // Compile shader

    ID3DXBuffer* shaderBuf = NULL;
    ID3DXBuffer* errBuf = NULL;

    // 使用 DXSDK (August 2008) 时报错:
    // error X3025: global variables are implicitly constant, enable compatibility mode to allow modification
    // 解决: 加入旗标 D3DXSHADER_USE_LEGACY_D3DX9_31_DLL 或 D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY
    ID3DXConstantTable* constTbl;
    hr = D3DXCompileShaderFromFile(VertexShaderFile, NULL, NULL,
                                   "Main", // entry point function name
                                   "vs_1_1", D3DXSHADER_DEBUG | D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, &shaderBuf, &errBuf, &constTbl);
    // output any error messages
    if (errBuf != NULL) {
        MYTRACEA("D3DXCompileShaderFromFile failed: %s", (char*) errBuf->GetBufferPointer());
        SGL::Release(errBuf);
    }
    SGL_FAILED_DO(hr, MYTRACE_DX("D3DXCompileShaderFromFile", hr); return FALSE);
    m_ToonConstTbl.Attach(constTbl);

    // Create shader
    IDirect3DVertexShader9* shader = NULL;
    hr = m_D3DDev->CreateVertexShader((DWORD*) shaderBuf->GetBufferPointer(), &shader);
    SGL_FAILED_DO(hr, MYTRACE_DX("CreateVertexShader", hr); return FALSE);
    m_ToonShader.Attach(shader);

    SGL::Release(shaderBuf);

    // Load textures.

    IDirect3DTexture9* tex;
    hr = D3DXCreateTextureFromFile(m_D3DDev, ToonShadeTexFile, &tex);
    SGL_FAILED_DO(hr, MYTRACE_DX("D3DXCreateTextureFromFile", hr); return FALSE);
    m_ShadeTex.Attach(tex);

    m_D3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    m_D3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    m_D3DDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

    // Get Handles

    m_WorldViewHandle       = constTbl->GetConstantByName(NULL, "WorldViewMatrix");
    m_WorldViewProjHandle   = constTbl->GetConstantByName(NULL, "WorldViewProjMatrix");
    m_ColorHandle           = constTbl->GetConstantByName(NULL, "Color");
    m_LightDirHandle        = constTbl->GetConstantByName(NULL, "LightDirection");

    // Set shader constants:

    // Light direction:
    D3DXVECTOR4 dirToLight(-0.57f, 0.57f, -0.57f, 0.0f);
    constTbl->SetVector(m_D3DDev, m_LightDirHandle, &dirToLight);
    constTbl->SetDefaults(m_D3DDev);

    // Compute projection matrix.
    D3DXMatrixPerspectiveFovLH(&m_ProjMatrix, D3DX_PI * 0.25f, (float) width / (float) height, 1.0f, 1000.0f);

    if (!InitFont())
        return FALSE;

    return TRUE;
}

BOOL RenderToon::InitFont() {
    // ID3DXFont 内部使用 GDI (DrawText) 绘制文字
    D3DXFONT_DESC fd;
    SGL::ZeroObj(&fd);
    fd.Height           = 20;   // in logical units
    fd.Width            = 7;    // in logical units
    fd.Weight           = 500;  // boldness, range 0(light) - 1000(bold)
    fd.MipLevels        = D3DX_DEFAULT;
    fd.Italic           = FALSE;
    fd.CharSet          = DEFAULT_CHARSET;
    fd.OutputPrecision  = 0;
    fd.Quality          = 0;
    fd.PitchAndFamily   = 0;
    _tcscpy_s(fd.FaceName, _countof(fd.FaceName), _T("Times New Roman"));

    // Create an ID3DXFont based on D3DXFONT_DESC.
    ID3DXFont* font;
    HRESULT hr = D3DXCreateFontIndirect(m_D3DDev, &fd, &font);
    SGL_FAILED_DO(hr, MYTRACE_DX("D3DXCreateFontIndirect", hr); return FALSE);
    m_Font.Attach(font);

    return TRUE;
}

void RenderToon::DumpMesh(LPCTSTR fname, ID3DXMesh* mesh) {
    MeshDump dump;
    if (!dump.Init(fname))
        return;
    dump.DumpVertex(mesh);
    dump.DumpIndex(mesh);
    dump.DumpAttributeBuffer(mesh);
    dump.DumpAttributeTable(mesh);
    dump.DumpAdjacencyBuffer(mesh);
}

void RenderToon::Draw(float timeDelta) {
    // Update the scene: Allow user to rotate around scene.
    static float angle  = (3.0f * D3DX_PI) / 2.0f;
    static float height = 5.0f;

    if (SGL::KEY_DOWN(VK_LEFT))
        angle -= 7.0f * timeDelta;
    if (SGL::KEY_DOWN(VK_RIGHT))
        angle += 7.0f * timeDelta;
    if (SGL::KEY_DOWN(VK_UP))
        height += 7.0f * timeDelta;
    if (SGL::KEY_DOWN(VK_DOWN))
        height -= 7.0f * timeDelta;

    D3DXVECTOR3 pos(cosf(angle) * 7.0f, height, sinf(angle) * 7.0f);
    D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    D3DXMATRIX V;
    D3DXMatrixLookAtLH(&V, &pos, &target, &up);

    // Render
    m_D3DDev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0xff, 0x40, 0x40, 0x40), 1.0f, 0);
    m_D3DDev->BeginScene();

    m_D3DDev->SetVertexShader(m_ToonShader);
    m_D3DDev->SetTexture(0, m_ShadeTex);

    D3DXMATRIX WorldView;
    D3DXMATRIX WorldViewProj;
    for (int i = 0; i < 4; ++i) {
        WorldView = m_WorldMatrices[i] * V;
        WorldViewProj = m_WorldMatrices[i] * V * m_ProjMatrix;

        m_ToonConstTbl->SetMatrix(m_D3DDev, m_WorldViewHandle, &WorldView);
        m_ToonConstTbl->SetMatrix(m_D3DDev, m_WorldViewProjHandle, &WorldViewProj);
        m_ToonConstTbl->SetVector(m_D3DDev, m_ColorHandle, &m_MeshColors[i]);

        m_Meshes[i]->DrawSubset(0);
    }

    // Draw FPS text
    RECT rect = {10, 10, m_Width, m_Height};
    m_Font->DrawText(NULL, m_FPS.CalcFPSStr(timeDelta), -1, &rect, DT_TOP | DT_LEFT, D3DCOLOR_ARGB(0xff, 0, 0x80, 0));

    m_D3DDev->EndScene();
    m_D3DDev->Present(NULL, NULL, NULL, NULL);
}

////////////////////////////////////////////////////////////////////////////////
// 输入处理类 ToonInput
////////////////////////////////////////////////////////////////////////////////

void ToonInput::ProcessInput(float timeDelta) {
    Input::QueryClose(VK_ESCAPE);
}

////////////////////////////////////////////////////////////////////////////////
// class MeshDump
////////////////////////////////////////////////////////////////////////////////

BOOL MeshDump::Init(LPCTSTR fname) {
    errno_t err = _tfopen_s(&m_DumpStream, fname, _T("w"));
    return (err == 0);
}

void MeshDump::Clean() {
    if (m_DumpStream != NULL) {
        fclose(m_DumpStream);
        m_DumpStream = NULL;
    }
}

void MeshDump::DumpVertex(ID3DXBaseMesh* mesh) {
    _fputts(_T("Vertex:\n---------\n"), m_DumpStream);
    Vertex* v = NULL;
    DWORD FVF = mesh->GetFVF();
    mesh->LockVertexBuffer(0, (void**) &v);
    size_t sz = Vertex::Size(FVF);
    v->DumpFVF(m_DumpStream, FVF);
    for (DWORD i = 0; i < mesh->GetNumVertices(); ++i) {
        _ftprintf_s(m_DumpStream, _T("Vertex %d:"), i);
        Vertex::Data(v, i, sz)->Dump(m_DumpStream, FVF);
    }

    mesh->UnlockVertexBuffer();
    _fputts(_T("\n"), m_DumpStream);
}

void MeshDump::DumpIndex(ID3DXBaseMesh* mesh) {
    _fputts(_T("Index:\n--------\n"), m_DumpStream);
    WORD* idx = NULL;
    mesh->LockIndexBuffer(0, (void**) &idx);
    if (idx == NULL)
        return;
    for (DWORD i = 0; i < mesh->GetNumFaces(); ++i)
        _ftprintf_s(m_DumpStream, _T("Triangle %d: %d, %d, %d\n"), i, idx[i * 3], idx[i * 3 + 1], idx[i * 3 + 2]);
    mesh->UnlockIndexBuffer();
    _fputts(_T("\n"), m_DumpStream);
}

// 基类 ID3DXBaseMesh 没有 LockAttributeBuffer/UnlockAttributeBuffer 方法
void MeshDump::DumpAttributeBuffer(ID3DXMesh* mesh) {
    _fputts(_T("Attribute Buffer:\n-----------------\n"), m_DumpStream);
    DWORD* attrBuf = NULL;
    mesh->LockAttributeBuffer(0, &attrBuf);
    // an attribute for each face
    for (DWORD i = 0; i < mesh->GetNumFaces(); ++i)
        _ftprintf_s(m_DumpStream, _T("Triangle lives in subset %d: %d\n"), i, attrBuf[i]);
    mesh->UnlockAttributeBuffer();
    _fputts(_T("\n"), m_DumpStream);
}

void MeshDump::DumpAdjacencyBuffer(ID3DXBaseMesh* mesh) {
    _fputts(_T("Adjacency Buffer:\n-----------------\n"), m_DumpStream);
    // three enttries per face
    std::vector<DWORD> adjBuf(mesh->GetNumFaces() * 3);
    mesh->GenerateAdjacency(0.0f, &adjBuf[0]);
    for (DWORD i = 0; i < mesh->GetNumFaces(); ++i)
        _ftprintf_s(m_DumpStream, _T("Triangle's adjacent to triangle %d: %d, %d, %d\n"), i, adjBuf[i * 3], adjBuf[i * 3 + 1], adjBuf[i * 3 + 2]);
    _fputts(_T("\n"), m_DumpStream);
}

void MeshDump::DumpAttributeTable(ID3DXBaseMesh* mesh) {
    _fputts(_T("Attribute Table:\n----------------\n"), m_DumpStream);
    // number of entries in the attribute table
    DWORD numEntries = 0;
    mesh->GetAttributeTable(0, &numEntries);
    if (numEntries == 0)
        return;
    std::vector<D3DXATTRIBUTERANGE> table(numEntries);
    mesh->GetAttributeTable(&table[0], &numEntries);
    for (DWORD i = 0; i < numEntries; ++i)
        _ftprintf_s(m_DumpStream,
                    _T("Entry %d\n-----------\n")
                    _T("Subset ID:    %d\n")
                    _T("Face Start:   %d\n")
                    _T("Face Count:   %d\n")
                    _T("Vertex Start: %d\n")
                    _T("Vertex Count: %d\n\n"),
                    i, table[i].AttribId, table[i].FaceStart, table[i].FaceCount, table[i].VertexStart, table[i].VertexCount);

    _fputts(_T("\n"), m_DumpStream);
}
