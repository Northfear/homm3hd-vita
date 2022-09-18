#include <vitasdk.h>
#include <vitaGL.h>
#include <imgui_vita.h>
#include <stdio.h>
#include <string>
#include <malloc.h>

extern "C" {
int _newlib_heap_size_user = 256 * 1024 * 1024;

void __real_free(void* ptr);
void *__real_malloc(size_t size);
void *__real_calloc(size_t num, size_t size);
void *__real_memalign(size_t alignment, size_t size);
void *__real_realloc(void *ptr, size_t size);

void *__wrap_memcpy(void *dest, const void *src, size_t n)
{
	return sceClibMemcpy(dest, src, n);
}

void *__wrap_memmove(void *dest, const void *src, size_t n)
{
	return sceClibMemmove(dest, src, n);
}

void *__wrap_memset(void *s, int c, size_t n)
{
	return sceClibMemset(s, c, n);
}

void __wrap_free(void* ptr)
{
	__real_free(ptr);
}

void *__wrap_malloc(size_t size)
{
	return __real_malloc(size);
}

void *__wrap_calloc(size_t num, size_t size)
{
	return __real_calloc(num, size);
}

void *__wrap_memalign(size_t alignment, size_t size)
{
	return __real_memalign(alignment, size);
}

void *__wrap_realloc(void *ptr, size_t size)
{
	return __real_realloc(ptr, size);
}
}

#define CONFIG_FILE_PATH "ux0:data/homm3hd/config.txt"

#define LANGUAGES_NUM 7
const char *LanguageName[LANGUAGES_NUM] = {
  "en",
  "fr",
  "it",
  "de",
  "es",
  "pl",
  "ru"
};

int language = 0;

void loadConfig(void) {
  char buffer[30];
  int value;

  FILE *config = fopen(CONFIG_FILE_PATH, "r");

  if (config) {
    while (EOF != fscanf(config, "%[^ ] %d\n", buffer, &value)) {
      if (strcmp("language", buffer) == 0) language = value;
    }
    fclose(config);
  }
}

void saveConfig(void) {
  FILE *config = fopen(CONFIG_FILE_PATH, "w+");

  if (config) {
    fprintf(config, "%s %d\n", "language", language);
    fclose(config);
  }
}

char *options_descs[] = {
  "Game language",
};

enum {
  OPT_LANGUAGE,
};

char *desc = nullptr;

void SetDescription(int i) {
  if (ImGui::IsItemHovered())
    desc = options_descs[i];
}

int main(int argc, char *argv[]) {
  loadConfig();
  int exit_code = 0xDEAD;

	vglInitExtended(0, 960, 544, 0x1800000, SCE_GXM_MULTISAMPLE_4X);

	// Setup ImGui binding
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplVitaGL_Init();

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui_ImplVitaGL_TouchUsage(true);
	ImGui_ImplVitaGL_UseIndirectFrontTouch(false);
	ImGui_ImplVitaGL_UseRearTouch(true);
	ImGui_ImplVitaGL_GamepadUsage(true);

  while (exit_code == 0xDEAD) {
    desc = nullptr;
    ImGui_ImplVitaGL_NewFrame();

    ImGui::SetNextWindowPos(ImVec2(160, 32), ImGuiSetCond_Always);
    ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiSetCond_Always);
    ImGui::Begin("##main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::TextColored(ImVec4(255, 255, 0, 255), "Game settings");
    ImGui::Text("Language:"); ImGui::SameLine();
    if (ImGui::BeginCombo("##combor", LanguageName[language])) {
      for (int n = 0; n < LANGUAGES_NUM; n++) {
        bool is_selected = language == n;
        if (ImGui::Selectable(LanguageName[n], is_selected))
          language = n;
        SetDescription(OPT_LANGUAGE);
        if (is_selected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }
    SetDescription(OPT_LANGUAGE);

    if (ImGui::Button("Save and Exit"))
      exit_code = 0;
    ImGui::SameLine();
    if (ImGui::Button("Save and Launch the game"))
      exit_code = 1;
    ImGui::SameLine();
    if (ImGui::Button("Discard and Exit"))
      exit_code = 2;
    ImGui::SameLine();
    if (ImGui::Button("Discard and Launch the game"))
      exit_code = 3;
    ImGui::Separator();

    if (desc) {
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::TextWrapped(desc);
    }

    ImGui::End();

		// Rendering
		glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui::Render();
		ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
		vglSwapBuffers(GL_FALSE);
  }

  if (exit_code < 2) // Save
    saveConfig();

  if (exit_code % 2 == 1) // Launch
    sceAppMgrLoadExec("app0:/eboot.bin", NULL, NULL);

  return 0;
}
