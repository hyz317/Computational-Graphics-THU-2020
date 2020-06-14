#include "sampler.hpp"
#include <iostream>

void Sampler::sampling() {

	for ( int i = 0 ; i < h ; i++ ) {
		for ( unsigned short j = 0, Xi[3] = {i, i*i, i*i*i} ; j < w ; j++ ) {
			// printf("%d %d\n", i, j);
			if (aperture < EPS) {
				Ray ray = camera->generateRay(Vector2f(j, i));
				Vector3f color = img->GetPixel(j, i);
				color += tracer->trace(ray, Xi, 1, i*w+j);
				img->SetPixel(j, i, color);
			} else {
				Vector3f color = img->GetPixel(j, i);
				for (int k = 0; k < dof_sample; k++) {
					Ray ray = camera->generateRandomRay(Vector2f(j, i), Xi);
					// TODO: might have bugs there.
					color += tracer->trace(ray, Xi, 1, i*w+j, Vector3f(1, 1, 1) / dof_sample) / dof_sample;
				}
				img->SetPixel(j, i, color);
			}

			
			if (j == w - 1) {
				printf("Sampling=%d/%d\n", i, h);
				if ((i & 7) == 0)
					printf("Stored hitpoints=%d\n", hitpointMap->GetStoredHitpoints());
			}
		}
	}
}

void Sampler::resampling() {
	
	for ( int i = 0 ; i < h ; i++ ) {
		for ( unsigned short j = 0, Xi[3] = {i, i*i, i*i*i} ; j < w ; j++ ) {
			// TODO: might have bugs there.
			Vector3f color = img->GetPixel(j, i) / 5;
			for ( int r = -1 ; r <= 1 ; r++ )
				for ( int c = -1 ; c <= 1 ; c++ ) {
					if (((r + c) & 1) == 0) continue;
					Ray ray = camera->generateRay(Vector2f((j + ( float ) c / 3.0f ), i + ( float ) r / 3.0f ));
					color += tracer->trace(ray, Xi, 1, i * w + j, Vector3f(1, 1, 1) / 5) / 5;
				}
			img->SetPixel(j, i, color);

			if (j == w - 1) {
				printf("Resampling=%d/%d\n", i, h);
				if ((i & 7) == 0)
					printf("Stored hitpoints=%d\n", hitpointMap->GetStoredHitpoints());
			}
		}
	}
}

void Sampler::ProgressivePhotonMapping(int SPPMIter) {
	int storedHitpoints = hitpointMap->GetStoredHitpoints();
	printf("Stored Hitpoints: %d\n", storedHitpoints);
	printf("HitpointMap Balancing...\n");
	hitpointMap->Balance();
	
	Vector3f** rtColor = new Vector3f*[h];
	for (int i = 0; i < h; i++) {
		rtColor[i] = new Vector3f[w];
		for (int j = 0; j < w; j++) {
			rtColor[i][j] = img->GetPixel(j, i);
			// std::cout << i << ' ' << j << ' ' << &rtColor[i][j] << std::endl;
		}
	}

	PhotonTracer* photontracer = new PhotonTracer(lights, camera->getPhotons(), 10, 1e-3);
	photontracer->SetHitpointMap(hitpointMap);
	photontracer->setGroup(group);
	for (int iter = 1; iter <= iterations; iter++) {
		printf("test0\n");

		// std::cout << 4 << ' ' << 0 << ' ' << &rtColor[4][0] << std::endl;

		photontracer->Emitting();
		// std::cout << "after emitting " << 4 << ' ' << 0 << ' ' << &rtColor[4][0] << std::endl;
		Hitpoint* hitpoints = hitpointMap->GetHitpoints();
		// std::cout << "after gethitpoints " << 4 << ' ' << 0 << ' ' << &rtColor[4][0] << std::endl;
		printf("test1\n");
		
		hitpointMap->MaintainHitpoints();
		// std::cout << "after maintain " << 4 << ' ' << 0 << ' ' << &rtColor[4][0] << std::endl;
		printf("test1.5\n");
		for (int r = 0; r < h; r++)
			for (int c = 0; c < w; c++) {
				// std::cout << "wtf " << 4 << ' ' << 0 << ' ' << &rtColor[4][0] << std::endl;
				img->SetPixel(c, r, rtColor[r][c]);
			}

		printf("test2\n");
		
		double minR2 = 1000.0f, maxR2 = 0;
		double minNum = 1000000000, maxNum = 0;
		for (int i = 1; i <= storedHitpoints; i++) {
			int r = hitpoints[i].rc / w;
			int c = hitpoints[i].rc % w;
			//std::cout << "getpixel: " << img->GetPixel(c, r) << " hitpoints[i].color:  " << hitpoints[i].color << " sth: " << hitpoints[i].weight << std::endl;
			Vector3f color = /*img->GetPixel(c, r) +*/ hitpoints[i].color * hitpoints[i].weight * (4.0 / (hitpoints[i].R2 * camera->getPhotons() * iter)) * 25000;
			img->SetPixel(c, r, color);
			
			if (hitpoints[i].R2 < minR2) minR2 = hitpoints[i].R2;
			if (hitpoints[i].R2 > maxR2) maxR2 = hitpoints[i].R2;
			if (hitpoints[i].num < minNum) minNum = hitpoints[i].num;
			if (hitpoints[i].num > maxNum) maxNum = hitpoints[i].num;
		}
		std::cout << "w=15,r=10: " << img->GetPixel(15, 10) << std::endl;
		printf("Iter=%d, Num=%.2lf~%.2lf, Radius=%.6lf~%.6lf\n", iter, minNum, maxNum, sqrt(minR2), sqrt(maxR2));
		img->SaveBMP("SPPMtest.bmp");
		
	}
	
	delete photontracer;
	for (int i = 0; i < h; i++)
		delete[] rtColor[i];
	delete[] rtColor;
}

void Sampler::start()
{
	for (int iter = 0; iter < dof_sample; iter++) {
		printf("SPPM Iteration= %d\n", iter);

		hitpointMap = new HitpointMap(4000000); // MaxHitPoints
		hitpointMap->SetReduction(0.7);
		tracer->setHitpointMap(hitpointMap);
		
		PhotonTracer* photontracer = new PhotonTracer(lights, camera->getPhotons(), 10, 1e-3);
		photontracer->setGroup(group);
		Photonmap* photonmap = photontracer->CalcPhotonmap();
		tracer->setPhotonMap(photonmap);
		delete photontracer;

		printf("photon map set up\n");

		randomsampling();

		printf("sampling finished\n");

		if (aperture < EPS) {
			int storedHitpoints = hitpointMap->GetStoredHitpoints();
			Hitpoint* hitpoints = hitpointMap->GetHitpoints();
			for (int i = 1; i <= storedHitpoints; i++) {
				int r = hitpoints[i].rc / w;
				int c = hitpoints[i].rc % w;
				hitpoints[i].weight *= 0.2;
			}
			// resampling();
		}
		
		img->SaveBMP("SPPMtest1.bmp");
		
		// ProgressivePhotonMapping(iter);
		
		if (hitpointMap != NULL) {
			delete hitpointMap;
			hitpointMap = NULL;
		}
		if (photonmap != NULL) {
			delete photonmap;
			photonmap = NULL;
		}
	}
}

void Sampler::randomsampling()
{
	int samps = 500;
	int multiThreadCounter = 0;
	#pragma omp parallel for
    for (int y = 0; y < h; y++) {
        // cout << "progress: " << (float) y / h * 100 << "%\n";
        for (unsigned short x = 0, Xi[3] = {y, y*y, y*y*y}; x < w; x++) {
            Vector3f ans(0, 0, 0);
            for (int sy = 0, i = (h-y-1) * w + x; sy < 2; sy++) {
                Vector3f r;
                for (int sx = 0; sx < 2; sx++, r = Vector3f::ZERO) {
                    for (int s = 0; s < samps; s++) {
                        double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1); 
                        double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2); 
                        // Ray ray = camera->generateRay(Vector2f((sx + 0.5 + dx) / 2 + x, (sy + 0.5 + dy) / 2 + y));
                        // Ray Tracing Todo
                        // ans += tracer->trace(ray, Xi, 1, 0) * 0.25 / samps;
						// std::cout << "new" << std::endl;

						if (aperture < EPS) {
							Ray ray = camera->generateRay(Vector2f((sx + 0.5 + dx) / 2 + x, (sy + 0.5 + dy) / 2 + y));
							ans += tracer->trace(ray, Xi, 1, 0) * 0.25 / samps;
						} else {
							for (int k = 0; k < dof_sample; k++) {
								unsigned short Xi2[3] = {k, y*y, x*y*k};
								Ray ray = camera->generateRandomRay(Vector2f((sx + 0.5 + dx) / 2 + x, (sy + 0.5 + dy) / 2 + y), Xi2);
								// TODO: might have bugs there.
								Vector3f what = tracer->trace(ray, Xi, 1, 0, Vector3f(1, 1, 1) / dof_sample) / dof_sample;
								// std::cout << what << std::endl;
								ans += what;
							}
						}
                    }
                }
            }
			ans *= (0.25 / samps);
			// std::cout << "ans " << ans << std::endl;
            img->SetPixel(x, y, ans);
        }
        multiThreadCounter++;
        std::cout << "progress: " << (float) multiThreadCounter / h * 100 << "%\n";
    }
}
