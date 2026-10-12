{
  "patcher": {
    "fileversion": 1,
    "appversion": {
      "major": 8,
      "minor": 5,
      "revision": 4,
      "architecture": "x64",
      "modernui": 1
    },
    "classnamespace": "box",
    "rect": [
      100,
      100,
      400,
      260
    ],
    "openrect": [
      0,
      0,
      0,
      169
    ],
    "openinpresentation": 1,
    "default_fontsize": 10,
    "default_fontname": "Arial Bold",
    "gridsize": [
      8,
      8
    ],
    "boxanimatetime": 500,
    "latency": 0,
    "is_mpe": 1,
    "minimum_live_version": "",
    "minimum_max_version": "",
    "platform_compatibility": 0,
    "project": {
      "version": 1,
      "creationdate": 3590052786,
      "modificationdate": 3590052786,
      "viewrect": [
        25,
        106,
        300,
        500
      ],
      "autoorganize": 1,
      "hideprojectwindow": 1,
      "showdependencies": 1,
      "autolocalize": 0,
      "contents": {
        "patchers": {}
      },
      "layout": {},
      "searchpath": {},
      "detailsvisible": 0,
      "amxdtype": 1835887981,
      "readonly": 0,
      "devpathtype": 0,
      "devpath": ".",
      "sortmode": 0,
      "viewmode": 0,
      "includepackages": 0
    },
    "autosave": 0,
    "boxes": [
      {
        "box": {
          "id": "obj-1",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            "int"
          ],
          "patching_rect": [
            24,
            24,
            40,
            20
          ],
          "text": "midiin"
        }
      },
      {
        "box": {
          "id": "obj-2",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            24,
            64,
            52,
            20
          ],
          "text": "js tap.js",
          "saved_object_attributes": {
            "filename": "tap.js",
            "parameter_enable": 0
          }
        }
      },
      {
        "box": {
          "id": "obj-3",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "outlettype": [],
          "patching_rect": [
            24,
            104,
            47,
            20
          ],
          "text": "midiout"
        }
      },
      {
        "box": {
          "id": "obj-10",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            24,
            160,
            120,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            4,
            120,
            18
          ],
          "text": "The Annoying Piano"
        }
      },
      {
        "box": {
          "id": "obj-11",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            24,
            184,
            120,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            24,
            120,
            18
          ],
          "text": "Swap mode",
          "fontname": "Arial"
        }
      }
    ],
    "lines": [
      {
        "patchline": {
          "destination": [
            "obj-2",
            0
          ],
          "source": [
            "obj-1",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-3",
            0
          ],
          "source": [
            "obj-2",
            0
          ]
        }
      }
    ],
    "parameters": {
      "inherited_shortname": 1
    }
  }
}
