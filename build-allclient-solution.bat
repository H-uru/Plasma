@echo off
REM **********************************************************************
REM *                                                                    *
REM * The CL options below define which client type you wish to          *
REM * build.  It is currently set for an external release live client.   *
REM *                                                                    *
REM **********************************************************************
SET CL=/DPLASMA_EXTERNAL_RELEASE /DBUILD_TYPE=BUILD_TYPE_LIVE
devenv.com .\MOULOpenSourceClientPlugin\Plasma20\MsDevProjects\Plasma\Apps\AllClient\AllClient.sln /build Release
